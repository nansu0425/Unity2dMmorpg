/*    DummyClient/Main.cpp    */

#include "DummyClient/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Network/Service.h"
#include "ServerEngine/Network/Session.h"

Byte gSendBuffer[] = "Hello, Server!";

class ServerSession
    : public Session
{
protected:
    virtual void    OnConnected() override
    {
        // 연결 성공 처리
        std::wcout << TEXT_16("Connected to server") << std::endl;
        // 서버에 데이터 전송
        Send(gSendBuffer, SIZE_64(gSendBuffer));
    }

    virtual void    OnDisconnected(String16 cause) override
    {
        // 연결 종료 처리
        std::wcout << TEXT_16("Disconnected: ") << cause << std::endl;
    }

    virtual Int64   OnReceived(Byte* buffer, Int64 numBytes) override
    {
        // 수신한 데이터 처리
        std::wcout << TEXT_16("Received: ") << numBytes << TEXT_16(" bytes") << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        // 수신한 데이터를 그대로 전송
        Send(buffer, numBytes);

        return numBytes;
    }

    virtual void    OnSent(Int64 numBytes) override
    {
        // 전송 완료 처리
        std::wcout << TEXT_16("Sent: ") << numBytes << TEXT_16(" bytes") << std::endl;
    }
};

Service::Config gConfig =
{
    NetAddress(TEXT_16("127.0.0.1"), 7777),
    std::make_shared<IoEventDispatcher>(),
    std::make_shared<ServerSession>,
    2,
};

int main()
{
    mi_version();

    // 서버 서비스 준비까지 잠시 대기
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 클라이언트 서비스 생성 및 실행
    auto service = std::make_shared<ClientService>(gConfig);
    ASSERT_CRASH(SUCCESS == service->Run(), "CLIENT_SERVICE_RUN_FAILED");

    // 입출력 이벤트 처리 스레드 생성 및 실행
    for (Int64 i = 0; i < 2; ++i)
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
    gThreadManager->Join();

    // 클라이언트 서비스 중지
    service->Stop();

    return 0;
}
