/*    ServerEngine/Thread/Thread.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Core/Thread.h"

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
    LockGuard guard(m_mutex);

    m_threads.emplace_back([callback]()
                           {
                               InitTls();
                               callback();
                               DestroyTls();
                           });
}

void ThreadManager::Join()
{
    LockGuard guard(m_mutex);

    for (Thread& thread : m_threads)
    {
        if (thread.joinable())
            thread.join();
    }

    m_threads.clear();
}

void ThreadManager::InitTls()
{
    static Atomic<Int32> s_threadId = 1;
    t_threadId = s_threadId.fetch_add(1);
}

void ThreadManager::DestroyTls()
{}
