/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "A.h"
#include "B.h"
#include "C.h"

A gA;
B gB;
C gC;

void ThreadA()
{
    while (true)
    {
        gA.WriteB(gB);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void ThreadB()
{
    while (true)
    {
        gB.WriteC(gC);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void ThreadC()
{
    while (true)
    {
        gC.WriteA(gA);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main()
{
    gThreadManager->Launch(ThreadA);
    gThreadManager->Launch(ThreadB);
    gThreadManager->Launch(ThreadC);

    gThreadManager->Join();

    return 0;
}
