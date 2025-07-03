/*    WorldServer/Main.cpp    */

#include "WorldServer/Pch.h"
#include "Core/Concurrency/Thread.h"
#include "Core/Io/Dispatcher.h"
#include "Core/Network/Service.h"
#include "WorldServer/Network/Session.h"
#include "GameLogic/Chat/Room.h"
#include "WorldServer/Packet/Handler.h"
#include "Protocol/Packet/Utils.h"
#include "GameLogic/Core/Loop.h"

using namespace core;
using namespace proto;
using namespace world;

Service::Config gConfig =
{
    NetAddress(TEXT_16("127.0.0.1"), 7777),
    std::make_shared<IoEventDispatcher>(),
    std::make_shared<ClientSession>,
    1000,
};

int main()
{
    // 서버 서비스 생성 및 실행
    auto service = std::make_shared<ServerService>(gConfig);
    ASSERT_CRASH(SUCCESS == service->Run(), "SERVER_SERVICE_RUN_FAILED");

    // 게임 루프 실행
    gThreadManager->Launch([]
                           {
                               game::Loop::GetInstance().Run(ToWorld_PacketHandler::GetInstance());
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

    // 룸 브로드캐스트 루프 실행
    WorldToClient_Chat chat;
    chat.set_id(0);
    chat.set_message(TEXT_8("Hello World!"));
    gRoom->StartBroadcastLoop(PacketUtils::MakeSendBuffer(chat, PacketId::WorldToClient_Chat), 100);

    gThreadManager->Join();

    // 서버 서비스 서비스 중지
    service->Stop();

    return 0;
}
