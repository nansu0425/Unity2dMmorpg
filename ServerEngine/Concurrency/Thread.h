/*    ServerEngine/Concurrency/Thread.h    */

#pragma once

class ThreadManager
{
private:
    template<typename T>
    using Vector = std::vector<T>;

public:
    ThreadManager();
    ~ThreadManager();

    void    Launch(Function<void(void)> callback);
    void    Join();

    static void     InitTls();
    static void     DestroyTls();

private:
    SRWLOCK             mLock;
    Vector<Thread>      mThreads;
};
