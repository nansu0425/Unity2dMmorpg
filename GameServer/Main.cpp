/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Network/Dispatcher.h"
#include "ServerEngine/Network/Service.h"
#include "GameServer/Network/Session.h"
#include "GameContent/Chat/Room.h"
#include "GameServer/Network/PacketHandler.h"

Service::Config gConfig =
{
    NetAddress(TEXT_16("127.0.0.1"), 7777),
    std::make_shared<IoEventDispatcher>(),
    std::make_shared<ClientSession>,
    100,
};

int main()
{
    // 서버 서비스 생성 및 실행
    auto service = std::make_shared<ServerService>(gConfig);
    ASSERT_CRASH(SUCCESS == service->Run(), "SERVER_SERVICE_RUN_FAILED");

    // 입출력 워커 실행
    for (Int64 i = 0; i < 3; ++i)
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

    // 룸 브로드캐스트 루프 실행
    S2C_Chat chat;
    chat.set_id(0);
    chat.set_message(TEXT_8("Hello World!"));
    gRoom->StartBroadcastLoop(PacketUtils::MakePacketBuffer(chat, PacketId::S2C_Chat), 100);

    gThreadManager->Join();

    // 서버 서비스 서비스 중지
    service->Stop();

    return 0;
}
