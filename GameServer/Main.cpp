/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "A.h"
#include "B.h"
#include "C.h"

//A gA;
//B gB;
//C gC;
//
//void ThreadA()
//{
//    while (true)
//    {
//        gA.WriteB(gB);
//        std::this_thread::sleep_for(std::chrono::milliseconds(10));
//    }
//}
//
//void ThreadB()
//{
//    while (true)
//    {
//        gB.WriteC(gC);
//        std::this_thread::sleep_for(std::chrono::milliseconds(10));
//    }
//}
//
//void ThreadC()
//{
//    while (true)
//    {
//        gC.WriteA(gA);
//        std::this_thread::sleep_for(std::chrono::milliseconds(10));
//    }
//}

#pragma optimize("", off)
Int32* gData;
RwSpinLock* gRwSpinLock;

void Write_RwSpinLock()
{
    for (Int32 i = 0; i < 1'000'000; ++i)
    {
        RwSpinLock::WriteGuard guard(*gRwSpinLock, "Write_RwSpinLock");
        ++*gData;
    }
}

void Read_RwSpinLock()
{
    // 실행 시간 측정 시작
    auto start = std::chrono::high_resolution_clock::now();
    while (true)
    {
        RwSpinLock::ReadGuard guard(*gRwSpinLock, "Read_RwSpinLock");
        if (*gData == 4'000'000)
        {
            break;
        }
    }
    // 실행 시간 측정 종료
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Read_RwSpinLock duration: " << duration.count() << " ms" << std::endl;
}
#pragma optimize("", on)

int main()
{
    gData = (Int32*)ALLOC_MEMORY(sizeof(Int32));
    gRwSpinLock = ::NewObject<RwSpinLock>();

    *gData = 0;
    gThreadManager->Launch(Read_RwSpinLock);
    for (Int32 i = 0; i < 4; ++i)
    {
        gThreadManager->Launch(Write_RwSpinLock);
    }
    gThreadManager->Join();
    
    ::DeleteObject(gRwSpinLock);
    FREE_MEMORY(gData);

    /*gThreadManager->Launch(ThreadA);
    gThreadManager->Launch(ThreadB);
    gThreadManager->Launch(ThreadC);

    gThreadManager->Join();*/

    return 0;
}
