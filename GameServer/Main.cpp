/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Network/Service.h"
#include "GameServer/Network/Session.h"
#include "GameServer/Network/SessionManager.h"
#include "GameServer/Network/Message.h"
#include "Common/Message.h"

#include <io.h>
#include <fcntl.h>

Service::Config gConfig =
{
    NetAddress(TEXT_16("127.0.0.1"), 7777),
    std::make_shared<IoEventDispatcher>(),
    std::make_shared<GameSession>,
    100,
};

int main()
{
    // 콘솔을 UTF-8 모드로 설정
    ::SetConsoleOutputCP(CP_UTF8);
    ::SetConsoleCP(CP_UTF8);
    // CRT stdout을 UTF-8 narrow-text 모드로 전환
    ::_setmode(::_fileno(stdout), _O_U8TEXT);

    // 모든 메시지 핸들러 등록
    gMessageHandlerManager.RegisterAllHandlers();

    // 서버 서비스 생성 및 실행
    auto service = std::make_shared<ServerService>(gConfig);
    ASSERT_CRASH(SUCCESS == service->Run(), "SERVER_SERVICE_RUN_FAILED");

    // 입출력 이벤트 처리 스레드 생성 및 실행
    for (Int64 i = 0; i < std::thread::hardware_concurrency() / 2; ++i)
    {
        gThreadManager->Launch([service]()
                               {
                                   Int64 result = SUCCESS;
                                   while (result == SUCCESS)
                                   {
                                       result = service->GetIoDispatcher()->Dispatch();
                                   }
                               });
    }

    while (true)
    {
        SharedPtr<SendMessageBuilder> message = std::make_shared<SendMessageBuilder>(MESSAGE_ID(ServerMessageId::Test));
        flatbuffers::FlatBufferBuilder& dataBuilder = message->GetDataBuilder();

        // buff 1
        Int64 victimsData1[] = {4000};
        flatbuffers::Offset<flatbuffers::Vector<Int64>> victims1 = dataBuilder.CreateVector(victimsData1, 1);
        flatbuffers::Offset<MessageData::Buff> buff1 = MessageData::CreateBuff(dataBuilder, 100, 1.2f, victims1);
        // buff 2
        Int64 victimsData2[] = {1000, 2000};
        flatbuffers::Offset<flatbuffers::Vector<Int64>> victims2 = dataBuilder.CreateVector(victimsData2, 2);
        flatbuffers::Offset<MessageData::Buff> buff2 = MessageData::CreateBuff(dataBuilder, 200, 2.5f, victims2);
        // test
        Vector<flatbuffers::Offset<MessageData::Buff>> buffData = {buff1, buff2};
        auto buffs = dataBuilder.CreateVector(buffData);
        auto test = MessageData::Server::CreateTest(dataBuilder, 1000, 100, 10, buffs);

        // Test 메시지를 모든 세션에 전송
        message->FinishBuilding(test);
        gSessionManager.Broadcast(std::move(message));

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    gThreadManager->Join();

    // 서버 서비스 서비스 중지
    service->Stop();

    return 0;
}
