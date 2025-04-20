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
    Atomic<Int32>   mPooledNodeCount = 0;
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
        kMaxBlockSize   = 4096,
    };

private:
    MemoryPool*         mPools[kPoolCount];
    MemoryPool*         mPoolTable[kMaxBlockSize + 1];
};

class alignas(MEMORY_ALLOCATION_ALIGNMENT) ChunkMemoryPool
{
public:
    struct alignas(MEMORY_ALLOCATION_ALIGNMENT) Node
        : public SLIST_ENTRY
    {
        Byte*   chunk = nullptr;
    };

    enum Config : Int64
    {
        kChunkSize          = 0x0001'0000,  // allocation granularity 배수로 설정
        kInitChunkCount     = 1024,
        kChargeChunkCount   = 16,
    };

public:
    ChunkMemoryPool();

    Node*   Pop();
    void    Push(Node* node);

private:
    void    AllocChunks(Int64 count);
    Node*   CreateNode();

private:
    SLIST_HEADER    mPooledNodes; // 스택 구조로 풀링하는 Chunk 노드 리스트
    Atomic<Int64>   mPooledNodeCount = 0;
    Atomic<Int64>   mTotalNodeCount = 0;
};

class BlockMemoryPool
{
public:
    using Node  = ChunkMemoryPool::Node;

    struct BlockHeader
    {
        BlockHeader*    next;
        Int64           blockSize;
    };

public:
    BlockMemoryPool(Int64 blockSize);
    ~BlockMemoryPool();

    BlockHeader*    Pop();
    void            Push(BlockHeader* header);

    Int64           GetBlockSize() const { return mBlockSize; }

private:
    void            ChargeBlocks();

private:
    const Int64     mBlockSize = 0;
    Node*           mPooledNodes = nullptr; // 청크 메모리 풀에서 가져온 노드를 스택 구조로 관리
    BlockHeader*    mPooledBlocks = nullptr; // 고정 크기 블록을 스택 구조로 관리
    
    Int64           mPooledNodeCount = 0;
    Int64           mPooledBlockCount = 0;
    Int64           mTotalBlockCount = 0;
};

class BlockMemoryPoolManager
{
public:
    enum Config : Int64
    {
        kMinBlockSize   = 32,
        kMaxBlockSize   = 1024,
    };

public:
    BlockMemoryPoolManager();
    ~BlockMemoryPoolManager();

    Byte*   Pop(Int64 allocSize);
    void    Push(Byte* block, Int64 allocSize);

private:
    using BlockHeader   = BlockMemoryPool::BlockHeader;

private:
    void    InitPools();

private:
    Vector<BlockMemoryPool*>    mBlockPools;
    BlockMemoryPool*            mSizeToPool[kMaxBlockSize + 1] = {};
};

struct alignas(MEMORY_ALLOCATION_ALIGNMENT) BlockMemoryHeader
    : public SLIST_ENTRY
{
    Int64   blockSize;
    Int64   poolIdx;
};

class alignas(MEMORY_ALLOCATION_ALIGNMENT) GlobalBlockMemoryPool
{
public:
    struct Config
    {
        Int64   blockSize;
        Int64   poolIdx;
        Int64   chunkSize;
        Int64   initChunkCount;
        Int64   chargeChunkCount;
    };

public:
    GlobalBlockMemoryPool(Config config);
    ~GlobalBlockMemoryPool();

    BlockMemoryHeader*      Pop();
    void                    Push(BlockMemoryHeader* header);

    Int64   GetBlockSize() const { return mConfig.blockSize; }

private:
    // 청크를 블록으로 나눠서 풀에 추가
    void    AddBlocks(Int64 chunkCount);
    // 연속된 청크들을 할당 받는다
    Byte*   AllocChunks(Int64 count);
    
private:
    SLIST_HEADER    mPooledBlocks = {}; // 락프리 싱글 연결 리스트를 스택 구조로 관리
    Config          mConfig;
    Atomic<Int64>   mPooledBlockCount = 0;
    Atomic<Int64>   mTotalBlockCount = 0;
};

class alignas(MEMORY_ALLOCATION_ALIGNMENT) GlobalBlockMemoryPoolManager
{
public:
    enum Config : Int64
    {
        kBlockPoolCount     = 6,
        kMinBlockSize       = 32,
        kMaxBlockSize       = 1024,
    };

public:
    GlobalBlockMemoryPoolManager();

    BlockMemoryHeader*      Pop(Int64 poolIdx);
    void                    Push(BlockMemoryHeader* header);

private:
    GlobalBlockMemoryPool   mBlockPools[kBlockPoolCount] =
    {
        {{ 32,   0, 0x0001'0000, 64, 16 }},
        {{ 64,   1, 0x0001'0000, 64, 16 }},
        {{ 128,  2, 0x0001'0000, 64, 16 }},
        {{ 256,  3, 0x0001'0000, 64, 16 }},
        {{ 512,  4, 0x0001'0000, 64, 16 }},
        {{ 1024, 5, 0x0001'0000, 64, 16 }},
    };
};

class TlsBlockMemoryPool
{
public:
    struct Config
    {
        Int64   blockSize;
        Int64   poolIdx;
        Int64   initBlockCount;
        Int64   chargeBlockCount;
    };

public:
    TlsBlockMemoryPool(Config config);
    ~TlsBlockMemoryPool();

    Byte*   Pop();
    void    Push(Byte* block);

    Int64   GetBlockSize() const { return mConfig.blockSize; }

private:
    void    AddBlocks(Int64 blockCount);

private:
    BlockMemoryHeader*      mPooledBlocks = nullptr; // 고정 크기 블록을 스택 구조로 관리
    Config                  mConfig;
    Int64                   mPooledBlockCount = 0;
};

class TlsBlockMemoryPoolManager
{
public:
    enum Config : Int64
    {
        kBlockPoolCount     = GlobalBlockMemoryPoolManager::kBlockPoolCount,
        kMinBlockSize       = GlobalBlockMemoryPoolManager::kMinBlockSize,
        kMaxBlockSize       = GlobalBlockMemoryPoolManager::kMaxBlockSize,
    };

public:
    TlsBlockMemoryPoolManager();

    Byte*       Pop(Int64 allocSize);
    void        Push(Byte* block, Int64 allocSize);

private:
    TlsBlockMemoryPool      mBlockPools[kBlockPoolCount] =
    {
        {{ 32,   0, 64, 16 }},
        {{ 64,   1, 64, 16 }},
        {{ 128,  2, 64, 16 }},
        {{ 256,  3, 64, 16 }},
        {{ 512,  4, 64, 16 }},
        {{ 1024, 5, 64, 16 }},
    };
    TlsBlockMemoryPool*     mSizeToPool[kMaxBlockSize + 1] = {};
};
