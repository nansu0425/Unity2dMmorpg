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

void ThreadManager::Launch(Function<void(void)> callback)
{
    SrwLockWriteGuard guard(mLock);

    mThreads.emplace_back([callback]()
                          {
                              InitTls();
                              callback();
                              DestroyTls();
                          });
}

void ThreadManager::Join()
{
    SrwLockWriteGuard guard(mLock);

    for (Thread& thread : mThreads)
    {
        if (thread.joinable())
            thread.join();
    }

    mThreads.clear();
}

void ThreadManager::InitTls()
{
    // 스레드 id 발급
    static Atomic<Int32> sThreadId = 1;
    tThreadId = sThreadId.fetch_add(1);
    ASSERT_CRASH(tThreadId > 0, "THREAD_ID_OVERFLOW");
    // 스레드 로컬 메모리 풀 생성
    tBlockMemoryPool = new BlockMemoryPool(64);
}

void ThreadManager::DestroyTls()
{
    // 스레드 로컬 메모리 풀 삭제
    delete tBlockMemoryPool;
    tBlockMemoryPool = nullptr;
    // 스레드 id 초기화
    tThreadId = 0;
}
