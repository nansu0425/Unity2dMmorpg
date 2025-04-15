/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"

class Resource
{
public:
    Resource(Int32 threadId)
        : mThreadId(::NewObject<Int32>(threadId))
    {
        std::cout << *mThreadId << ": Resource created" << std::endl;
    }
    ~Resource()
    {
        std::cout << *mThreadId << ": Resource destroyed" << std::endl;
        ::DeleteObject(mThreadId);
    }

private:
    Int32* mThreadId = nullptr;
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
        {
            WRITE_GUARD;
            mData.push_back(tThreadId);
            threadId = mData.back();
        }
        while (true)
        {
            Resource* resoucre = ObjectPool<Resource>::Pop(threadId);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            ObjectPool<Resource>::Push(resoucre);
        }
    }

private:
    RW_LOCK;
    Vector<Int32> mData;
};

int main()
{
    Workers* workers = ObjectPool<Workers>::Pop(4);
    ObjectPool<Workers>::Push(workers);

    return 0;
}
