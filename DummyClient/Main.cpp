/*    DummyClient/Main.cpp    */

#include "DummyClient/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Network/Service.h"
#include "ServerEngine/Network/Session.h"
#include "DummyClient/Network/Packet.h"

Char8 gMessage[4096] = {};

class ServerSession
    : public PacketSession
{
protected:
    virtual void    OnConnected() override
    {
        gLogger->Info(TEXT_16("Connected to server"));
    }

    virtual void    OnDisconnected(String16 cause) override
    {
        gLogger->Warn(TEXT_16("Disconnected from server: {}"), cause);
    }

    virtual void    OnPacketReceived(Byte* packet, Int64 size) override
    {
        ServerPacketHandler::HandlePacket(packet, size);
    }

    virtual void    OnSent(Int64 numBytes) override
    {}
};

Service::Config gConfig =
{
    NetAddress(TEXT_16("127.0.0.1"), 7777),
    std::make_shared<IoEventDispatcher>(),
    std::make_shared<ServerSession>,
    1,
};

int main()
{
    // 서버 서비스 준비까지 잠시 대기
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 클라이언트 서비스 생성 및 실행
    auto service = std::make_shared<ClientService>(gConfig);
    ASSERT_CRASH(SUCCESS == service->Run(), "CLIENT_SERVICE_RUN_FAILED");

    // 입출력 이벤트 처리 스레드 생성 및 실행
    for (Int64 i = 0; i < 4; ++i)
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
