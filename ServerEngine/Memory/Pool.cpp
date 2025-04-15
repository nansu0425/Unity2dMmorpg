/*    ServerEngine/Memory/Pool.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Memory/Pool.h"

MemoryPool::MemoryPool(UInt64 allocSize)
    : mAllocSize(allocSize)
{}

MemoryPool::~MemoryPool()
{
    while (!mHeaders.empty())
    {
        MemoryHeader* header = mHeaders.front();
        mHeaders.pop();
        ::free(header);
    }
}

void MemoryPool::Push(MemoryHeader* header)
{
    ASSERT_CRASH(header->allocSize == mAllocSize, "INVALID_MEMORY_HEADER");
    {
        WRITE_GUARD;
        mHeaders.push(header);
    }
    mAllocCount.fetch_sub(1);
}

MemoryHeader* MemoryPool::Pop()
{
    MemoryHeader* header = nullptr;
    {
        WRITE_GUARD;
        // 여분의 헤더가 있는 경우
        if (!mHeaders.empty())
        {
            header = mHeaders.front();
            mHeaders.pop();
        }
    }
    // 여분의 헤더가 없는 경우
    if (header == nullptr)
    {
        // 메모리 할당
        header = static_cast<MemoryHeader*>(::malloc(mAllocSize));
        header->allocSize = mAllocSize;
    }
    ASSERT_CRASH(header->allocSize == mAllocSize, "INVALID_MEMORY_HEADER");
    mAllocCount.fetch_add(1);

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
    const UInt64 allocSize = header->allocSize;

    if (allocSize > kMaxAllocSize)
    {
        // 최대 할당 크기보다 큰 경우
        ::free(header);
    }
    else
    {
        // 메모리 풀에 반납
        mPoolTable[allocSize]->Push(header);
    }
}

void* MemoryPoolManager::Pop(UInt64 size)
{
    MemoryHeader* header = nullptr;
    const UInt64 allocSize = sizeof(MemoryHeader) + size;
    
    if (allocSize > kMaxAllocSize)
    {
        // 최대 할당 크기보다 큰 경우
        header = static_cast<MemoryHeader*>(::malloc(allocSize));
        header->allocSize = allocSize;
    }
    else
    {
        // 메모리 풀에서 가져온다
        header = mPoolTable[allocSize]->Pop();
    }

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
