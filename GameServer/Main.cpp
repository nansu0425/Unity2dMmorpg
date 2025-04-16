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

int main()
{
    auto workers = PoolObjectAllocator::MakeUnique<Workers>(4);
    PoolObjectAllocator::UniquePtr<Int32> data = nullptr;

    return 0;
}
