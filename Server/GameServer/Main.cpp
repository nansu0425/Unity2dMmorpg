/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "Core/Concurrency/Thread.h"
#include "Core/Io/Dispatcher.h"
#include "Core/Network/Service.h"
#include "GameServer/Network/Session.h"
#include "GameServer/Chat/Room.h"
#include "GameServer/Packet/Handler.h"
#include "Protocol/Packet/Utils.h"
#include "GameServer/Core/Loop.h"

core::Service::Config gConfig =
{
    core::NetAddress(TEXT_16("127.0.0.1"), 7777),
    std::make_shared<core::IoEventDispatcher>(),
    std::make_shared<game::ClientSession>,
    1000,
};

int main()
{
    // 서버 서비스 생성 및 실행
    auto service = std::make_shared<core::ServerService>(gConfig);
    ASSERT_CRASH(SUCCESS == service->Run(), "SERVER_SERVICE_RUN_FAILED");

    // 게임 루프 실행
    core::gThreadManager->Launch([]
                           {
                               game::Loop::GetInstance().Run();
                           });

    // 입출력 워커 실행
    for (Int64 i = 0; i < 4; ++i)
    {
        core::gThreadManager->Launch([service]
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
        core::gThreadManager->Launch([]
                               {
                                   core::gJobQueueManager->FlushQueues();
                               });
    }

    // 잡 타이머 실행
    core::gThreadManager->Launch([]
                           {
                               core::gJobTimer->Run();
                           });

    // 룸 브로드캐스트 루프 실행
    proto::WorldToClient_Chat chat;
    chat.set_id(0);
    chat.set_message(TEXT_8("Hello World!"));
    game::gRoom->StartBroadcastLoop(proto::PacketUtils::MakeSendBuffer(chat, proto::PacketId::WorldToClient_Chat), 100);

    core::gThreadManager->Join();

    // 서버 서비스 서비스 중지
    service->Stop();

    return 0;
}
