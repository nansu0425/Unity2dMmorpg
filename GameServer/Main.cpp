/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"

class ResourceBase
{
public:
    ResourceBase(Int32 threadId)
        : mThreadId(ObjectPool<Int32>::Pop(threadId))
    {
        std::cout << *mThreadId << ": Resource created\n";
    }
    ~ResourceBase()
    {
        std::cout << *mThreadId << ": Resource destroyed\n";
        ObjectPool<Int32>::Push(mThreadId);
    }

protected:
    Int32* mThreadId = 0;
};

class Resource
    : public ResourceBase
{
public:
    Resource(Int32 threadId, UInt64 workerNumber)
        : ResourceBase(threadId)
        , mWorkerNumber(ObjectPool<UInt64>::Pop(workerNumber))
    {
        std::cout << *mThreadId << ": worker number is " << *mWorkerNumber << "\n";
    }

    ~Resource()
    {
        ObjectPool<UInt64>::Push(mWorkerNumber);
    }

private:
    UInt64* mWorkerNumber = nullptr;
};

class Workers
{
public:
    Workers(const Int32 workerCount)
    {
        for (Int32 i = 0; i < workerCount; ++i)
        {
            gThreadManager->Launch([this]()
                                   {
                                       Run();
                                   });
        }
    }

    ~Workers()
    {
        gThreadManager->Join();
    }

private:
    void Run()
    {
        Int32 threadId = 0;
        UInt64 workerNumber = 0;
        {
            WRITE_GUARD;
            mData.push_back(tThreadId);
            threadId = mData.back();
            workerNumber = mData.size();
        }
        while (true)
        {
            PoolObjectAllocator::SharedPtr<ResourceBase> resoucre1 = PoolObjectAllocator::MakeShared<Resource>(threadId, workerNumber);
            PoolObjectAllocator::UniquePtr<ResourceBase> resoucre2 = PoolObjectAllocator::MakeUnique<Resource>(threadId, workerNumber);
        }
    }

private:
    RW_LOCK;
    PoolMemoryAllocator::Vector<Int32> mData;
};

constexpr Int32 gTestCount = 1000;
constexpr Int32 gDataSize = 1000;

void TestMemory_std()
{
    for (Int32 i = 0; i < gTestCount; ++i)
    {
        Vector<UniquePtr<Int32>> data(gDataSize);
        for (Int32 j = 0; j < gDataSize; ++j)
        {
            data[j] = std::make_unique<Int32>(j);
        }
    }
}

void TestMemory_custom()
{
    for (Int32 i = 0; i < gTestCount; ++i)
    {
        PoolMemoryAllocator::Vector<PoolObjectAllocator::UniquePtr<Int32>> data(gDataSize);
        for (Int32 j = 0; j < gDataSize; ++j)
        {
            data[j] = PoolObjectAllocator::MakeUnique<Int32>(j);
        }
    }
}

void BlockMemoryPoolTest()
{
    Byte* blocks[10'000] = {};

    for (Int32 i = 0; i < 1000; ++i)
    {
        for (Int32 i = 0; i < 10'000; ++i)
        {
            blocks[i] = tBlockMemoryPool->Pop();
        }
        for (Int32 i = 0; i < 10'000; ++i)
        {
            tBlockMemoryPool->Push(blocks[i]);
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
            blocks[i] = static_cast<Byte*>(::malloc(64));
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
    //    gThreadManager->Launch(TestMemory_std);
    //}
    //gThreadManager->Join();
    //// 실행 시간 측정 종료
    //auto end = std::chrono::high_resolution_clock::now();
    //// ms 단위 실행 시간 계산
    //auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    //std::cout << "TestMemory_std: " << duration.count() << " ms\n";
    //
    //// 실행 시간 측정 시작
    //start = std::chrono::high_resolution_clock::now();
    //for (Int32 i = 0; i < 4; ++i)
    //{
    //    gThreadManager->Launch(TestMemory_custom);
    //}
    //gThreadManager->Join();
    //// 실행 시간 측정 종료
    //end = std::chrono::high_resolution_clock::now();
    //// ms 단위 실행 시간 계산
    //duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    //std::cout << "TestMemory_custom: " << duration.count() << " ms\n";

    // 실행 시간 측정 시작
    auto start = std::chrono::high_resolution_clock::now();
    for (Int32 i = 0; i < 4; ++i)
    {
        gThreadManager->Launch(BlockMemoryPoolTest);
    }
    gThreadManager->Join();
    // 실행 시간 측정 종료
    auto end = std::chrono::high_resolution_clock::now();
    // ms 단위 실행 시간 계산
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "BlockMemoryPoolTest: " << duration.count() << " ms\n";

    // 실행 시간 측정 시작
    start = std::chrono::high_resolution_clock::now();
    for (Int32 i = 0; i < 4; ++i)
    {
        gThreadManager->Launch(MallocTest);
    }
    gThreadManager->Join();
    // 실행 시간 측정 종료
    end = std::chrono::high_resolution_clock::now();
    // ms 단위 실행 시간 계산
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "MallocTest: " << duration.count() << " ms\n";

    return 0;
}
