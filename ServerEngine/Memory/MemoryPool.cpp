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
    mPoolingCount.fetch_add(1);
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
        mPoolingCount.fetch_sub(1);
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
    ASSERT_CRASH(allocSize == kMaxAllocSize + 1, "INVALID_ALLOC_MAPPING");
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
    if (allocSize > kMaxAllocSize)
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
    if (allocSize > kMaxAllocSize)
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

MemoryChunkPool::MemoryChunkPool()
{
    ::InitializeSListHead(&mChunkNodePool);
    AddChunkNodes(AllocChunks(kInitChunkCount), kInitChunkCount);
}

Byte* MemoryChunkPool::Pop()
{
    Node* node = static_cast<Node*>(::InterlockedPopEntrySList(&mChunkNodePool));
    // 노드가 없는 경우
    if (node == nullptr)
    {
        // 청크 생성 및 노드 추가
        AddChunkNodes(AllocChunks(kRefillChunkCount), kRefillChunkCount);
        node = static_cast<Node*>(::InterlockedPopEntrySList(&mChunkNodePool));
        ASSERT_CRASH(node != nullptr, "SLIST_EMPTY");
    }
    mChunkNodeCount.fetch_sub(1);
    Byte* chunk = node->chunk;
    // 청크 정보를 제거하고 노드를 풀링
    node->chunk = nullptr;
    ::InterlockedPushEntrySList(&mEmptyNodePool, node);
    mEmptyNodeCount.fetch_add(1);

    return chunk;
}

void MemoryChunkPool::Push(Byte* chunk)
{
    Node* node = static_cast<Node*>(::InterlockedPopEntrySList(&mEmptyNodePool));
    if (node == nullptr)
    {
        node = CreateNode();
    }
    else
    {
        mEmptyNodeCount.fetch_sub(1);
    }
    node->chunk = chunk;
    // 청크 노드 풀링
    ::InterlockedPushEntrySList(&mChunkNodePool, node);
    mChunkNodeCount.fetch_add(1);
}

void MemoryChunkPool::AddChunkNodes(Byte* chunks, Int64 count)
{
    for (Int64 i = 0; i < count; ++i)
    {
        // 노드 생성
        Node* node = CreateNode();
        node->chunk = chunks + (i * kChunkSize);
        // 노드 추가
        ::InterlockedPushEntrySList(&mChunkNodePool, node);
        mChunkNodeCount.fetch_add(1);
    }
}

Byte* MemoryChunkPool::AllocChunks(Int64 count)
{
    Byte* chunks = static_cast<Byte*>(::VirtualAlloc(nullptr, kChunkSize * count, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    ASSERT_CRASH(chunks != nullptr, "VIRTUAL_ALLOC_FAILED");
    mTotalChunkCount.fetch_add(count);

    return chunks;
}

MemoryChunkPool::Node* MemoryChunkPool::CreateNode()
{
    Node* node = static_cast<Node*>(::_aligned_malloc(sizeof(Node), MEMORY_ALLOCATION_ALIGNMENT));
    ASSERT_CRASH(node != nullptr, "ALIGNED_MALLOC_FAILED");

    return node;
}
