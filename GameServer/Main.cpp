/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"

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
    for (Int32 i = 0; i < 5; ++i)
    {
        gThreadManager->Launch(ThreadMain);
    }

    gThreadManager->Join();

    return 0;
}
