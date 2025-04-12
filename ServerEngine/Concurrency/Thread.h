/*    ServerEngine/Concurrency/Thread.h    */

#pragma once

class ThreadManager
{
public:
    ThreadManager();
    ~ThreadManager();

    void    Launch(Func<void(void)> callback);
    void    Join();

    static void     InitTls();
    static void     DestroyTls();

private:
    Mutex               mMutex;
    Vector<Thread>      mThreads;
};
