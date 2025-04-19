/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"

class Resource
{
public:
    static void* operator new(size_t size)
    {
        return BlockMemoryAllocator::Alloc(size);
    }

    static void operator delete(void* memory) noexcept
    {
        BlockMemoryAllocator::Free(memory, sizeof(Resource));
    }

private:
    UInt64   mA = 0xAAAA'AAAA;
    UInt64   mB = 0xBBBB'BBBB;
    UInt64   mC = 0xCCCC'CCCC;
    UInt64   mD = 0xDDDD'DDDD;
};

void BlockMemoryPoolTest()
{
    Byte* blocks[10'000] = {};
    UInt64 size[10'000] = {};

    for (Int32 i = 0; i < 1000; ++i)
    {
        for (Int32 i = 0; i < 10'000; ++i)
        {
            // 1~1024 범위 내에서 랜덤한 크기 할당
            // size[i] = (rand() % 1024) + 1;
            blocks[i] = static_cast<Byte*>(BlockMemoryAllocator::Alloc(800));
        }
        for (Int32 i = 0; i < 10'000; ++i)
        {
            BlockMemoryAllocator::Free(blocks[i], 800);
        }
    }
}

void MallocTest()
{
    Byte* blocks[10'000] = {};

    for (Int32 i = 0; i < 1000; ++i)
    {
        for (Int32 i = 0; i < 10'000; ++i)
        {
            // 1~1024 범위 내에서 랜덤한 크기 할당
            // UInt64 size = (rand() % 1024) + 1;
            blocks[i] = static_cast<Byte*>(::malloc(800));
        }
        for (Int32 i = 0; i < 10'000; ++i)
        {
            ::free(blocks[i]);
        }
    }
}

int main()
{
    //// 실행 시간 측정 시작
    //auto start = std::chrono::high_resolution_clock::now();
    //for (Int32 i = 0; i < 4; ++i)
    //{
    //    gThreadManager->Launch(BlockMemoryPoolTest);
    //}
    //gThreadManager->Join();
    //// 실행 시간 측정 종료
    //auto end = std::chrono::high_resolution_clock::now();
    //// ms 단위 실행 시간 계산
    //auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    //std::cout << "BlockMemoryPoolTest: " << duration.count() << " ms\n";

    //// 실행 시간 측정 시작
    //start = std::chrono::high_resolution_clock::now();
    //for (Int32 i = 0; i < 4; ++i)
    //{
    //    gThreadManager->Launch(MallocTest);
    //}
    //gThreadManager->Join();
    //// 실행 시간 측정 종료
    //end = std::chrono::high_resolution_clock::now();
    //// ms 단위 실행 시간 계산
    //duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    //std::cout << "MallocTest: " << duration.count() << " ms\n";

    Resource* resource = new Resource();
    delete resource;

    return 0;
}
