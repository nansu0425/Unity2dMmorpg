/*    ServerEngine/Concurrency/Thread.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"

ThreadManager::ThreadManager()
{
    InitTls();
}

ThreadManager::~ThreadManager()
{
    Join();
}

void ThreadManager::Launch(Func<void(void)> callback)
{
    LockGuard guard(mMutex);

    mThreads.emplace_back([callback]()
                           {
                               InitTls();
                               callback();
                               DestroyTls();
                           });
}

void ThreadManager::Join()
{
    LockGuard guard(mMutex);

    for (Thread& thread : mThreads)
    {
        if (thread.joinable())
            thread.join();
    }

    mThreads.clear();
}

void ThreadManager::InitTls()
{
    static Atomic<Int16> sThreadId = 1;
    tThreadId = sThreadId.fetch_add(1);

    ASSERT_CRASH(tThreadId > 0);
}

void ThreadManager::DestroyTls()
{}
