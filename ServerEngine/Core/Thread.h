/*    ServerEngine/Thread/Thread.h    */

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
    Mutex               m_mutex;
    Vector<Thread>      m_threads;
};
