﻿/*    DummyClient/Main.cpp    */

#include "DummyClient/Pch.h"
#include "Core/Concurrency/Thread.h"
#include "Core/Io/Dispatcher.h"
#include "Core/Network/Service.h"
#include "DummyClient/Network/Session.h"
#include "DummyClient/Packet/Handler.h"
#include "DummyClient/Core/Loop.h"

using namespace core;
using namespace dummy;

Service::Config gConfig =
{
    NetAddress(TEXT_16("127.0.0.1"), 7777),
    std::make_shared<IoEventDispatcher>(),
    std::make_shared<ServerSession>,
    1000,
};

int main()
{
    // 서버 서비스 준비까지 잠시 대기
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    // 클라이언트 서비스 생성 및 실행
    auto service = std::make_shared<ClientService>(gConfig);
    ASSERT_CRASH(SUCCESS == service->Run(), "CLIENT_SERVICE_RUN_FAILED");

    // 더미 클라이언트 루프 실행
    gThreadManager->Launch([]
                           {
                               dummy::Loop::GetInstance().Run();
                           });

    // 입출력 워커 실행
    for (Int64 i = 0; i < 4; ++i)
    {
        gThreadManager->Launch([service]
                               {
                                   while (true)
                                   {
                                       Int64 result = service->GetIoEventDispatcher()->Dispatch();
                                   }
                               });
    }

    // 잡 워커 실행
    for (Int64 i = 0; i < 3; ++i)
    {
        gThreadManager->Launch([]
                               {
                                   gJobQueueManager->FlushQueues();
                               });
    }

    // 잡 타이머 실행
    gThreadManager->Launch([]
                           {
                               gJobTimer->Run();
                           });


    gThreadManager->Join();

    // 클라이언트 서비스 중지
    service->Stop();

    return 0;
}
