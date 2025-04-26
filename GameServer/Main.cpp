/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Network/Service.h"
#include "GameServer/Network/Session.h"
#include "GameServer/Network/SessionManager.h"

Service::Config gConfig =
{
    NetAddress(TEXT_16("127.0.0.1"), 7777),
    std::make_shared<IoEventDispatcher>(),
    std::make_shared<GameSession>,
    100,
};

const Byte gSendData[] = TEXT_8("Hello, World!");

int main()
{
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
        SharedPtr<SendBuffer> sendBuffer = gSendBufferManager->Open(4096);
        // 송신 패킷 헤더 설정
        PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->GetBuffer());
        header->size = SIZE_32(PacketHeader) + SIZE_32(gSendData);
        header->id = 0x0001;
        // 송신 데이터 복사
        ::memcpy(header + 1, gSendData, SIZE_64(gSendData));
        sendBuffer->Close(header->size);
        // 송신 데이터를 모든 세션에 브로드캐스트
        gSessionManager.Broadcast(sendBuffer);

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    gThreadManager->Join();

    // 서버 서비스 서비스 중지
    service->Stop();

    return 0;
}
