/*    ServerEngine/Memory/MemoryPool.h    */

#pragma once

/*
 * [|     MemoryHeader     ][|                   Memory                   ]|
 *  |header                  |header + sizeof(MemoryHeader)                |header + allocSize
 */
struct alignas(MEMORY_ALLOCATION_ALIGNMENT) MemoryHeader
    : public SLIST_ENTRY
{
    UInt64      allocSize;
    void*       ownerMemory = nullptr;

    // 메모리 포인터 반환
    static void* AttachHeader(MemoryHeader* header)
    {
        header->ownerMemory = header + 1;
        return header->ownerMemory;
    }

    // 헤더 포인터 반환
    static MemoryHeader* DetachHeader(void* memory)
    {
        MemoryHeader* header = reinterpret_cast<MemoryHeader*>(memory) - 1;
        header->ownerMemory = nullptr;
        return header;
    }
};

class alignas(MEMORY_ALLOCATION_ALIGNMENT) MemoryPool
{
public:
    MemoryPool(UInt64 allocSize);
    ~MemoryPool();

    void            Push(MemoryHeader* header);
    MemoryHeader*   Pop();

private:
    SLIST_HEADER    mHeaders; // MemoryHeader 타입 메모리를 스택으로 관리
    UInt64          mAllocSize = 0;
    Atomic<Int32>   mUseCount = 0;
    Atomic<Int32>   mPoolingCount = 0;
};

/*
 * 1 ~ 1024B 할당     : 32, 64, ... , 1024B 풀 사용
 * 1025 ~ 2048B 할당  : 1152, 1280, ... , 2048B 풀 사용
 * 2049 ~ 4096B 할당  : 2304, 2560, ... , 4096B 풀 사용
 */
class MemoryPoolManager
{
public:
    MemoryPoolManager();
    ~MemoryPoolManager();

    void    Push(void* memory);
    void*   Pop(UInt64 size);
    
private:
    void    InitPools(Int32 startPoolSize, Int32 endPoolSize, Int32 poolSize, Int32& poolIdx, Int32& allocSize);

private:
    template<typename T>
    using Vector        = std::vector<T>;

    enum : Int32
    {
        kPoolCount      = (1024 / 32) + (1024 / 128) + (2048 / 256),
        kMaxAllocSize   = 4096,
    };

private:
    MemoryPool*         mPools[kPoolCount];
    MemoryPool*         mPoolTable[kMaxAllocSize + 1];
};

class alignas(MEMORY_ALLOCATION_ALIGNMENT) MemoryChunkPool
{
public:
    struct alignas(MEMORY_ALLOCATION_ALIGNMENT) Node
        : public SLIST_ENTRY
    {
        Byte* chunk = nullptr;
    };

    enum Config : Int64
    {
        kChunkSize = 64 * 1024,
        kInitChunkCount = 16,
        kRefillChunkCount = 4,
    };

public:
    MemoryChunkPool();

    Byte*   Pop();
    void    Push(Byte* chunk);

private:
    void    AddChunkNodes(Byte* chunks, Int64 count);
    Byte*   AllocChunks(Int64 count);
    Node*   CreateNode();

private:
    SLIST_HEADER    mChunkNodePool; // Chunk 포인터를 갖고 있는 노드 풀
    SLIST_HEADER    mEmptyNodePool; // Chunk를 정보를 갖고 있지 않은 노드 풀
    Atomic<Int64>   mChunkNodeCount = 0;
    Atomic<Int64>   mEmptyNodeCount = 0;
    Atomic<Int64>   mTotalChunkCount = 0;
};
