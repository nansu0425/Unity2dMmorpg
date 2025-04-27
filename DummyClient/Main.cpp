/*    DummyClient/Main.cpp    */

#include "DummyClient/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Network/Service.h"
#include "ServerEngine/Network/Session.h"

Char8 gReceiveBuffer[4096] = {};

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

    virtual Int64   OnPacketReceived(Byte* buffer, Int64 numBytes) override
    {
        PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
        // 수신 버퍼에 복사
        ::memcpy(gReceiveBuffer, header + 1, header->size - SIZE_32(PacketHeader));
        gLogger->Debug(TEXT_8("Received packet: size={}, id={}, data={}"), header->size, header->id, gReceiveBuffer);

        return numBytes;
    }

    virtual void    OnSent(Int64 numBytes) override
    {}
};

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
