/*    DummyClient/Main.cpp    */

#include "DummyClient/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Network/Service.h"
#include "ServerEngine/Network/Session.h"
#include "DummyClient/Network/Message.h"

#include <io.h>
#include <fcntl.h>

class ServerSession
    : public Session
{
protected:
    virtual void    OnConnected() override
    {
        gLogger->Info(TEXT_8("Connected to server"));
        
        auto message = std::make_shared<SendMessageBuilder>(MESSAGE_ID(ClientMessageId::Login));
        flatbuffers::FlatBufferBuilder& dataBuilder = message->GetDataBuilder();

        auto id = dataBuilder.CreateString(TEXT_8("hello1234"));
        auto password = dataBuilder.CreateString(TEXT_8("987abcd"));
        auto login = MessageData::Client::CreateLogin(dataBuilder, id, password);

        // 로그인 메시지 전송
        message->FinishBuilding(login);
        Send(std::move(message));
    }

    virtual void    OnDisconnected(String8 cause) override
    {
        gLogger->Warn(TEXT_8("Disconnected from server: {}"), cause);
    }

    virtual void    OnReceived(ReceiveMessage message) override
    {
        gMessageHandlerManager.HandleMessage(GetSharedPtr(), message);
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
    // 콘솔을 UTF-8 모드로 설정
    ::SetConsoleOutputCP(CP_UTF8);
    ::SetConsoleCP(CP_UTF8);
    // CRT stdout을 UTF-8 narrow-text 모드로 전환
    ::_setmode(::_fileno(stdout), _O_U8TEXT);

    // 서버 서비스 준비까지 잠시 대기
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    // 모든 메시지 핸들러 등록
    gMessageHandlerManager.RegisterAllHandlers();

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
