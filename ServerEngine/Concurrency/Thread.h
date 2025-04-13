/*    ServerEngine/Concurrency/Thread.h    */

#pragma once

class ThreadManager
{
public:
    ThreadManager();
    ~ThreadManager();

    void    Launch(Function<void(void)> callback);
    void    Join();

    static void     InitTls();
    static void     DestroyTls();

private:
    SharedMutex         mLock;
    Vector<Thread>      mThreads;
};
