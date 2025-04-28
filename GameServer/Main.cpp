/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Network/Service.h"
#include "GameServer/Network/Session.h"
#include "GameServer/Network/SessionManager.h"
#include "GameServer/Network/Packet.h"
#include <io.h>
#include <fcntl.h>

Service::Config gConfig =
{
    NetAddress(TEXT_16("127.0.0.1"), 7777),
    std::make_shared<IoEventDispatcher>(),
    std::make_shared<GameSession>,
    1000,
};

const Byte gMessage[] = TEXT_8("Hello, World!");

int main()
{
    // 콘솔을 UTF-8 모드로 설정
    ::SetConsoleOutputCP(CP_UTF8);
    ::SetConsoleCP(CP_UTF8);
    // CRT stdout을 UTF-8 narrow-text 모드로 전환
    ::_setmode(::_fileno(stdout), _O_U8TEXT);

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
        Vector<Buff> buffs{ { 1, 10.1f }, { 2, 20.45f }, { 3, 30.234f } };
        // Test 패킷을 모든 세션에 전송
        SharedPtr<SendBuffer> sendBuffer = ServerPacketGenerator::GenerateTestPacket(1001, 100, 10, std::move(buffs), TEXT_16("안녕하세요"));
        gSessionManager.Broadcast(sendBuffer);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    gThreadManager->Join();

    // 서버 서비스 서비스 중지
    service->Stop();

    return 0;
}
