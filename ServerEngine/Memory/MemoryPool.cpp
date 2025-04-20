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

ChunkMemoryPool::ChunkMemoryPool()
{
    ::InitializeSListHead(&mPooledNodes);
    AllocChunks(kInitChunkCount);
}

ChunkMemoryPool::Node* ChunkMemoryPool::Pop()
{
    Node* node = static_cast<Node*>(::InterlockedPopEntrySList(&mPooledNodes));
    // 노드가 없는 경우
    if (node == nullptr)
    {
        // 부족한 노드를 채운다
        AllocChunks(kChargeChunkCount);
        node = static_cast<Node*>(::InterlockedPopEntrySList(&mPooledNodes));
        ASSERT_CRASH(node != nullptr, "POOLED_NODE_EMPTY");
    }
    node->Next = nullptr;
    mPooledNodeCount.fetch_sub(1);

    return node;
}

void ChunkMemoryPool::Push(Node* node)
{
#ifdef _DEBUG
    ::memset(node->chunk, 0x00, kChunkSize);
#endif // _DEBUG
    ASSERT_CRASH_DEBUG(node->Next == nullptr, "NODE_CORRUPTION");
    ::InterlockedPushEntrySList(&mPooledNodes, node);
    mPooledNodeCount.fetch_add(1);
}

void ChunkMemoryPool::AllocChunks(Int64 count)
{
    // 연속된 청크를 할당 받는다
    Byte* chunks = static_cast<Byte*>(::VirtualAlloc(nullptr, kChunkSize * count, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    ASSERT_CRASH(chunks != nullptr, "VIRTUAL_ALLOC_FAILED");
    mTotalNodeCount.fetch_add(count);

    // 연속된 청크들을 쪼개서 노드에 할당
    for (Int64 i = 0; i < count; ++i)
    {
        // 노드 생성
        Node* node = CreateNode();
        node->chunk = chunks + (i * kChunkSize);
        // 노드 추가
        ::InterlockedPushEntrySList(&mPooledNodes, node);
        mPooledNodeCount.fetch_add(1);
    }
}

ChunkMemoryPool::Node* ChunkMemoryPool::CreateNode()
{
    Node* node = static_cast<Node*>(::_aligned_malloc(sizeof(Node), MEMORY_ALLOCATION_ALIGNMENT));
    ASSERT_CRASH(node != nullptr, "ALIGNED_MALLOC_FAILED");

    return node;
}

BlockMemoryPool::BlockMemoryPool(Int64 blockSize)
    : mBlockSize(blockSize)
{
    ASSERT_CRASH((blockSize > 0) && (ChunkMemoryPool::kChunkSize % blockSize == 0), "INVALID_BLOCK_SIZE");
    ChargeBlocks();
}

BlockMemoryPool::~BlockMemoryPool()
{
    while (Node* node = mPooledNodes)
    {
        // 노드 반납
        mPooledNodes = static_cast<Node*>(node->Next);
        node->Next = nullptr;
        --mPooledNodeCount;
        gChunkMemoryPool->Push(node);
    }
}

BlockMemoryPool::BlockHeader* BlockMemoryPool::Pop()
{
    // 풀에서 블록을 꺼낸다
    BlockHeader* header = mPooledBlocks;
    if (header == nullptr)
    {
        ChargeBlocks();
        header = mPooledBlocks;
        ASSERT_CRASH(header != nullptr, "POOLED_BLOCKS_EMPTY");
    }

    ASSERT_CRASH_DEBUG(header->blockSize == mBlockSize, "INVALID_BLOCK");
    mPooledBlocks = header->next;
    header->next = nullptr;
    --mPooledBlockCount;

    return header;
}

void BlockMemoryPool::Push(BlockHeader* header)
{
#ifdef _DEBUG
    ::memset(header, 0x00, mBlockSize);
#endif // _DEBUG
    
    header->next = mPooledBlocks;
    header->blockSize = mBlockSize;
    mPooledBlocks = header;
    ++mPooledBlockCount;
}

void BlockMemoryPool::ChargeBlocks()
{
    // 청크 메모리 풀에서 가져온 노드를 풀에 넣는다
    Node* node = gChunkMemoryPool->Pop();
    node->Next = mPooledNodes;
    mPooledNodes = node;
    ++mPooledNodeCount;

    // 청크를 블록으로 나누어 풀에 추가
    for (Int64 i = 0; i < ChunkMemoryPool::kChunkSize / mBlockSize; ++i)
    {
        // 헤더 설정
        BlockHeader* header = reinterpret_cast<BlockHeader*>(node->chunk + (i * mBlockSize));
        header->next = mPooledBlocks;
        header->blockSize = mBlockSize;

        mPooledBlocks = header;
        ++mPooledBlockCount;
        ++mTotalBlockCount;
    }
}

BlockMemoryPoolManager::BlockMemoryPoolManager()
{
    InitPools();
}

BlockMemoryPoolManager::~BlockMemoryPoolManager()
{
    for (BlockMemoryPool* pool : mBlockPools)
    {
        delete pool;
        pool = nullptr;
    }
}

Byte* BlockMemoryPoolManager::Pop(Int64 allocSize)
{
    BlockHeader* header = nullptr;

    if (allocSize > kMaxBlockSize)
    {
        // 최대 블록 크기보다 큰 경우 메모리 할당
        header = static_cast<BlockHeader*>(::_aligned_malloc(allocSize, 64));
    }
    else
    {
        header = mSizeToPool[allocSize]->Pop();
        ASSERT_CRASH_DEBUG(header->blockSize >= allocSize, "INVALID_BLOCK");
        header->blockSize = 0;
    }
    
    return reinterpret_cast<Byte*>(header);
}

void BlockMemoryPoolManager::Push(Byte* block, Int64 allocSize)
{
    BlockHeader* header = reinterpret_cast<BlockHeader*>(block);

    if (allocSize > kMaxBlockSize)
    {
        // 최대 블록 크기보다 큰 경우 메모리 해제
        ::_aligned_free(header);
    }
    else
    {
        mSizeToPool[allocSize]->Push(header);
    }
}

void BlockMemoryPoolManager::InitPools()
{
    Int64 blockSize = kMinBlockSize;
    Int64 allocSize = 0;

    while (blockSize <= kMaxBlockSize)
    {
        // 블록 풀 생성
        mBlockPools.push_back(new BlockMemoryPool(blockSize));
        // 할당 크기에 맞는 블록 풀에 매핑
        for (; allocSize <= blockSize; ++allocSize)
        {
            mSizeToPool[allocSize] = mBlockPools.back();
        }

        blockSize <<= 1;
    }
}

GlobalBlockMemoryPool::GlobalBlockMemoryPool(Config config)
    : mConfig(config)
{
    ASSERT_CRASH((mConfig.chunkSize % 0x0001'0000 == 0) && // 청크의 크기는 64KB의 배수여야 한다
                 (mConfig.chunkSize % mConfig.blockSize == 0) && // 청크의 크기는 블록 크기로 나누어 떨어져야 한다
                 (mConfig.blockSize >= SIZE_64(BlockMemoryHeader)), // 블록 크기는 헤더 크기보다 커야 한다
                 "INVALID_CONFIG");

    ::InitializeSListHead(&mPooledBlocks);
    AddBlocks(mConfig.initChunkCount);
}

GlobalBlockMemoryPool::~GlobalBlockMemoryPool()
{}

BlockMemoryHeader* GlobalBlockMemoryPool::Pop()
{
    BlockMemoryHeader* header = static_cast<BlockMemoryHeader*>(::InterlockedPopEntrySList(&mPooledBlocks));

    if (header == nullptr)
    {
        // 블록이 없는 경우 블록 추가
        AddBlocks(mConfig.chargeChunkCount);
        header = static_cast<BlockMemoryHeader*>(::InterlockedPopEntrySList(&mPooledBlocks));
        ASSERT_CRASH(header != nullptr, "POOLED_BLOCKS_EMPTY");
    }

    ASSERT_CRASH_DEBUG((header->blockSize == mConfig.blockSize) &&
                       (header->poolIdx == mConfig.poolIdx),
                       "INVALID_BLOCK");
    mPooledBlockCount.fetch_sub(1);

    return header;
}

void GlobalBlockMemoryPool::Push(BlockMemoryHeader* header)
{
    ASSERT_CRASH_DEBUG((header->blockSize == mConfig.blockSize) &&
                       (header->poolIdx == mConfig.poolIdx),
                       "INVALID_BLOCK");
    ::InterlockedPushEntrySList(&mPooledBlocks, header);
    mPooledBlockCount.fetch_add(1);
}

void GlobalBlockMemoryPool::AddBlocks(Int64 chunkCount)
{
    Byte* chunks = AllocChunks(chunkCount);
    const Int64 blockCount = chunkCount * mConfig.chunkSize / mConfig.blockSize;
    Byte* block = chunks;

    // 청크를 블록으로 나누어 풀에 추가
    for (Int64 i = 0; i < blockCount; ++i)
    {
        // 헤더 설정
        BlockMemoryHeader* header = reinterpret_cast<BlockMemoryHeader*>(block);
        header->blockSize = mConfig.blockSize;
        header->poolIdx = mConfig.poolIdx;
        // 블록 추가
        ::InterlockedPushEntrySList(&mPooledBlocks, reinterpret_cast<BlockMemoryHeader*>(block));
        mPooledBlockCount.fetch_add(1);
        mTotalBlockCount.fetch_add(1);
        block += mConfig.blockSize;
    }
}

Byte* GlobalBlockMemoryPool::AllocChunks(Int64 count)
{
    // 연속된 청크를 할당 받는다
    Byte* chunks = static_cast<Byte*>(::VirtualAlloc(nullptr, mConfig.chunkSize * count, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    ASSERT_CRASH(chunks != nullptr, "VIRTUAL_ALLOC_FAILED");

    return chunks;
}

GlobalBlockMemoryPoolManager::GlobalBlockMemoryPoolManager()
{}

BlockMemoryHeader* GlobalBlockMemoryPoolManager::Pop(Int64 poolIdx)
{
    ASSERT_CRASH_DEBUG((0 <= poolIdx) &&
                       (poolIdx < kBlockPoolCount),
                       "INVALID_POOL_INDEX");
    return mBlockPools[poolIdx].Pop();
}

void GlobalBlockMemoryPoolManager::Push(BlockMemoryHeader* header)
{
    ASSERT_CRASH_DEBUG((0 <= header->poolIdx) &&
                       (header->poolIdx < kBlockPoolCount),
                       "INVALID_POOL_INDEX");
    mBlockPools[header->poolIdx].Push(header);
}

TlsBlockMemoryPool::TlsBlockMemoryPool(Config config)
    : mConfig(config)
{
    AddBlocks(mConfig.initBlockCount);
}

TlsBlockMemoryPool::~TlsBlockMemoryPool()
{
    while (BlockMemoryHeader* header = mPooledBlocks)
    {
        // 블록 반납
        mPooledBlocks = static_cast<BlockMemoryHeader*>(header->Next);
        header->Next = nullptr;
        --mPooledBlockCount;
        gBlockMemoryPoolManager->Push(header);
    }
    ASSERT_CRASH(mPooledBlockCount == 0, "INVALID_BLOCK_COUNT");
}

Byte* TlsBlockMemoryPool::Pop()
{
    BlockMemoryHeader* header = mPooledBlocks;

    if (header == nullptr)
    {
        // 블록이 없는 경우 블록 추가
        AddBlocks(mConfig.chargeBlockCount);
        header = mPooledBlocks;
        ASSERT_CRASH(header != nullptr, "POOLED_BLOCKS_EMPTY");
    }
    ASSERT_CRASH_DEBUG((header->blockSize == mConfig.blockSize) &&
                       (header->poolIdx == mConfig.poolIdx),
                       "INVALID_BLOCK");
    mPooledBlocks = static_cast<BlockMemoryHeader*>(header->Next);
    header->Next = nullptr;
    --mPooledBlockCount;
#ifdef _DEBUG
    ::memset(header, 0x00, SIZE_64(BlockMemoryHeader));
#endif // _DEBUG

    return reinterpret_cast<Byte*>(header);
}

void TlsBlockMemoryPool::Push(Byte* block)
{
    BlockMemoryHeader* header = reinterpret_cast<BlockMemoryHeader*>(block);
#ifdef _DEBUG
    ::memset(header, 0x00, mConfig.blockSize);
#endif // _DEBUG

    header->Next = mPooledBlocks;
    header->blockSize = mConfig.blockSize;
    header->poolIdx = mConfig.poolIdx;

    mPooledBlocks = header;
    ++mPooledBlockCount;
}

void TlsBlockMemoryPool::AddBlocks(Int64 blockCount)
{
    Int64 prevCount = mPooledBlockCount;

    for (Int64 i = 0; i < blockCount; ++i)
    {
        // 전역 블록 풀에서 블록을 가져온다
        BlockMemoryHeader* header = gBlockMemoryPoolManager->Pop(mConfig.poolIdx);
        ASSERT_CRASH_DEBUG((header->blockSize == mConfig.blockSize) &&
                           (header->poolIdx == mConfig.poolIdx),
                           "INVALID_BLOCK");
        // 블록 추가
        header->Next = mPooledBlocks;
        mPooledBlocks = header;
        ++mPooledBlockCount;
    }

    ASSERT_CRASH_DEBUG((mPooledBlockCount - prevCount) == blockCount, "INVALID_BLOCK_COUNT");
}

TlsBlockMemoryPoolManager::TlsBlockMemoryPoolManager()
{
    Int64 allocSize = 1;
    Int64 blockSize = kMinBlockSize;

    for (Int64 i = 0; i < kBlockPoolCount; ++i)
    {
        ASSERT_CRASH(blockSize == mBlockPools[i].GetBlockSize(), "INVALID_BLOCK_SIZE");
        // 할당 크기에 맞는 블록 풀 매핑
        for (; allocSize <= blockSize; ++allocSize)
        {
            mSizeToPool[allocSize] = &mBlockPools[i];
        }
        blockSize <<= 1;
    }
}

Byte* TlsBlockMemoryPoolManager::Pop(Int64 allocSize)
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

void TlsBlockMemoryPoolManager::Push(Byte* block, Int64 allocSize)
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
