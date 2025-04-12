/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Core/Thread.h"

void ThreadMain()
{
    while (true)
    {
        std::cout << "Thread ID: " << t_threadId << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main()
{
    for (Int32 i = 0; i < 5; ++i)
    {
        g_threadManager->Launch(ThreadMain);
    }

    g_threadManager->Join();

    return 0;
}
