/*    ServerEngine/Memory/MemoryPool.cpp    */

#include "ServerEngine/Pch.h"

MemoryPool::MemoryPool(UInt64 allocSize)
    : mAllocSize(allocSize)
{
    ::InitializeSListHead(&mHeaders);
}

MemoryPool::~MemoryPool()
{
    while (MemoryHeader* header = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&mHeaders)))
    {
        ::_aligned_free(header);
    }
}

void MemoryPool::Push(MemoryHeader* header)
{
    ASSERT_CRASH_DEBUG(header->allocSize == mAllocSize, "INVALID_MEMORY_HEADER");
    // 헤더 반납
    ::InterlockedPushEntrySList(&mHeaders, header);
    mUseCount.fetch_sub(1);
    mPooledNodeCount.fetch_add(1);
}

MemoryHeader* MemoryPool::Pop()
{
    MemoryHeader* header = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&mHeaders));
    // 여분의 헤더가 없는 경우
    if (header == nullptr)
    {
        // 메모리 할당
        header = static_cast<MemoryHeader*>(::_aligned_malloc(mAllocSize, MEMORY_ALLOCATION_ALIGNMENT));
        header->allocSize = mAllocSize;
    }
    else
    {
        mPooledNodeCount.fetch_sub(1);
    }
    ASSERT_CRASH_DEBUG(header->allocSize == mAllocSize, "INVALID_MEMORY_HEADER");
    mUseCount.fetch_add(1);

    return header;
}

MemoryPoolManager::MemoryPoolManager()
{
    Int32 poolIdx = 0;
    Int32 allocSize = 1;

    // 4096B 풀까지 생성
    InitPools(32, 1024, 32, poolIdx, allocSize);
    InitPools(1024 + 128, 2048, 128, poolIdx, allocSize);
    InitPools(2048 + 256, 4096, 256, poolIdx, allocSize);

    ASSERT_CRASH(poolIdx == kPoolCount, "INVALID_POOL_COUNT");
    ASSERT_CRASH(allocSize == kMaxBlockSize + 1, "INVALID_ALLOC_MAPPING");
}

MemoryPoolManager::~MemoryPoolManager()
{
    for (MemoryPool* pool : mPools)
    {
        delete pool;
        pool = nullptr;
    }
}

void MemoryPoolManager::Push(void* memory)
{
    MemoryHeader* header = MemoryHeader::DetachHeader(memory);
#if USE_STOMP_ALLOCATOR
    StompMemoryAllocator::Free(header);
#else
    const UInt64 allocSize = header->allocSize;
    if (allocSize > kMaxBlockSize)
    {
        // 최대 할당 크기보다 큰 경우 메모리 해제
        ::_aligned_free(header);
    }
    else
    {
        // 메모리 풀에 반납
        mPoolTable[allocSize]->Push(header);
    }
#endif // USE_STOMP_ALLOCATOR
}

void* MemoryPoolManager::Pop(UInt64 size)
{
    MemoryHeader* header = nullptr;
    const UInt64 allocSize = sizeof(MemoryHeader) + size;
#if USE_STOMP_ALLOCATOR
    header = static_cast<MemoryHeader*>(StompMemoryAllocator::Alloc(allocSize));
#else
    if (allocSize > kMaxBlockSize)
    {
        // 최대 할당 크기보다 큰 경우 메모리 할당
        header = static_cast<MemoryHeader*>(::_aligned_malloc(allocSize, MEMORY_ALLOCATION_ALIGNMENT));
        header->allocSize = allocSize;
    }
    else
    {
        // 메모리 풀에서 가져온다
        header = mPoolTable[allocSize]->Pop();
    }
#endif // USE_STOMP_ALLOCATOR
    return MemoryHeader::AttachHeader(header);
}

void MemoryPoolManager::InitPools(Int32 startPoolSize, Int32 endPoolSize, Int32 poolSize, Int32& poolIdx, Int32& allocSize)
{
    for (Int32 maxAllocSize = startPoolSize; maxAllocSize <= endPoolSize; maxAllocSize += poolSize)
    {
        mPools[poolIdx] = new MemoryPool(maxAllocSize);
        // 풀 테이블에 메모리 풀을 매핑
        while (allocSize <= maxAllocSize)
        {
            mPoolTable[allocSize] = mPools[poolIdx];
            ++allocSize;
        }
        ++poolIdx;
    }
}

BlockMemoryPool::BlockMemoryPool(Int64 blockSize)
    : mBlockSize(blockSize)
{
    AllocBlocks(kInitBlockCount);
}

Byte* BlockMemoryPool::Pop()
{
    BlockHeader* header = mPooledBlocks;

    if (header == nullptr)
    {
        // 블록이 없는 경우 블록 추가
        AllocBlocks(kChargeBlockCount);
        header = mPooledBlocks;
        ASSERT_CRASH(header != nullptr, "POOLED_BLOCKS_EMPTY");
    }
    ASSERT_CRASH((header->canary == kBlockCanary) &&
                 (header->size == mBlockSize),
                 "INVALID_BLOCK");
    mPooledBlocks = header->next;
    --mPooledBlockCount;

    header->canary = 0;
    header->next = nullptr;
    header->size = 0;

    return reinterpret_cast<Byte*>(header);
}

void BlockMemoryPool::Push(Byte* block)
{
#ifdef _DEBUG
    ::memset(block, 0x00, mBlockSize);
#endif // _DEBUG
    BlockHeader* header = reinterpret_cast<BlockHeader*>(block);
    header->canary = kBlockCanary;
    header->next = mPooledBlocks;
    header->size = mBlockSize;

    mPooledBlocks = header;
    ++mPooledBlockCount;
}

void BlockMemoryPool::AllocBlocks(Int64 count)
{
    // 연속된 블록을 할당 받는다
    Byte* blocks = static_cast<Byte*>(::_aligned_malloc(mBlockSize * count, MEMORY_ALLOCATION_ALIGNMENT));
    ASSERT_CRASH(blocks != nullptr, "MALLOC_FAILED");
#ifdef _DEBUG
    ::memset(blocks, 0x00, mBlockSize * count);
#endif // _DEBUG
    mAllocBlockCount += count;

    // 연속된 블록들을 쪼개서 풀에 추가
    for (Int64 i = 0; i < count; ++i)
    {
        // 헤더 설정
        BlockHeader* header = reinterpret_cast<BlockHeader*>(blocks + (i * mBlockSize));
        header->canary = kBlockCanary;
        header->next = mPooledBlocks;
        header->size = mBlockSize;
        // 블록 추가
        mPooledBlocks = header;
        ++mPooledBlockCount;
    }
}

BlockMemoryPoolManager::BlockMemoryPoolManager()
{
    Int64 allocSize = 1;
    mBlockPools.reserve(kPoolCount);

    InitPools(32, 1024, 32, allocSize);
    InitPools(1024 + 128, 2048, 128, allocSize);
    InitPools(2048 + 256, 4096, 256, allocSize);
    ASSERT_CRASH(allocSize == kMaxBlockSize + 1, "INVALID_ALLOC_MAPPING");
}

Byte* BlockMemoryPoolManager::Pop(Int64 allocSize)
{
    Byte* block = nullptr;

    if (allocSize > kMaxBlockSize)
    {
        // 최대 블록 크기보다 큰 경우 메모리 할당
        block = static_cast<Byte*>(::_aligned_malloc(allocSize, MEMORY_ALLOCATION_ALIGNMENT));
    }
    else
    {
        // 블록 풀에서 블록을 가져온다
        block = mSizeToPool[allocSize]->Pop();
    }

    return block;
}

void BlockMemoryPoolManager::Push(Byte* block, Int64 allocSize)
{
    if (allocSize > kMaxBlockSize)
    {
        // 최대 블록 크기보다 큰 경우 메모리 해제
        ::_aligned_free(block);
    }
    else
    {
        // 블록 풀에 반납
        mSizeToPool[allocSize]->Push(block);
    }
}

void BlockMemoryPoolManager::InitPools(Int64 startBlockSize, Int64 endBlockSize, Int64 stepBlockSize, Int64& allocSize)
{
    for (Int64 blockSize = startBlockSize; blockSize <= endBlockSize; blockSize += stepBlockSize)
    {
        mBlockPools.emplace_back(blockSize);
        // 할당 크기에 맞는 블록 풀 매핑
        while (allocSize <= blockSize)
        {
            mSizeToPool[allocSize] = &mBlockPools.back();
            ++allocSize;
        }
    }
}
