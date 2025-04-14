/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"

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
        
        std::cout << "Worker thread ID: " << threadId << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Worker finished" << std::endl;
    }

private:
    RW_LOCK;
    Vector<Int32> mData;
};

int main()
{
    Workers* workers = ::NewObject<Workers>(4);
    ::DeleteObject(workers);

    return 0;
}
