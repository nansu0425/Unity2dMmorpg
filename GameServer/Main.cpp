/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"

class ResourceBase
{
public:
    ResourceBase(Int32 threadId)
        : mThreadId(::NewObject<Int32>(threadId))
    {
        std::cout << *mThreadId << ": Resource created\n";
    }
    ~ResourceBase()
    {
        std::cout << *mThreadId << ": Resource destroyed\n";
        ::DeleteObject(mThreadId);
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
        , mWorkerNumber(::NewObject<UInt64>(workerNumber))
    {
        std::cout << *mThreadId << ": worker number is " << *mWorkerNumber << "\n";
    }

    ~Resource()
    {
        ::DeleteObject(mWorkerNumber);
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
            ObjectPool<ResourceBase>::UniquePtr resoucre = ObjectPool<Resource>::MakeUnique(threadId, workerNumber);
        }
    }

private:
    RW_LOCK;
    Vector<Int32> mData;
};

int main()
{
    ObjectPool<Workers>::SharedPtr workers = ObjectPool<Workers>::MakeShared(4);
    ObjectPool<Int32>::SharedPtr data = nullptr;

    return 0;
}
