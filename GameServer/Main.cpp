/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"

class TestLock
{
public:
    Int32 TestRead()
    {
        READ_GUARD;

        if (mQueue.empty())
        {
            return -1;
        }

        return mQueue.front();
    }

    void TestPush()
    {
        WRITE_GUARD;

        mQueue.push(rand() % 100);
    }

    void TestPop()
    {
        WRITE_GUARD;

        if (!mQueue.empty())
        {
            mQueue.pop();
        }
    }

private:
    RW_SPIN_LOCK;
    std::queue<Int32> mQueue;
};

TestLock gTestLock;

void WriteThread()
{
    while (true)
    {
        gTestLock.TestPush();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        gTestLock.TestPop();
    }
}

void ReadThread()
{
    while (true)
    {
        Int32 value = gTestLock.TestRead();
        if (value != -1)
        {
            std::cout << "Read Value: " << value << std::endl;
        }
        else
        {
            std::cout << "Queue is empty" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void ThreadMain()
{
    while (true)
    {
        std::cout << "Thread ID: " << tThreadId << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main()
{
    for (Int32 i = 0; i < 2; ++i)
    {
        gThreadManager->Launch(WriteThread);
    }

    for (Int32 i = 0; i < 4; ++i)
    {
        gThreadManager->Launch(ReadThread);
    }

    gThreadManager->Join();

    return 0;
}
