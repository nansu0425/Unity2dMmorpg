/*    DummyClient/Main.cpp    */

#include "DummyClient/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Network/Service.h"
#include "ServerEngine/Network/Session.h"
#include "DummyClient/Network/Message.h"

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
    100,
};

int main()
{
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

    // 채팅 메시지 빌드
    auto sendMessage = std::make_shared<SendMessageBuilder>(MESSAGE_ID(ClientMessageId::Chat));
    auto& dataBuilder = sendMessage->GetDataBuilder();
    auto chatMessage = dataBuilder.CreateString(TEXT_8("Hello, world!"));
    auto dataChat = MessageData::Client::CreateChat(dataBuilder, chatMessage);
    sendMessage->FinishBuilding(dataChat);

    // 서비스의 세션들에 브로드캐스트
    while (true)
    {
        // 메시지 전송
        service->Broadcast(sendMessage); 
        // 1초 대기
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    gThreadManager->Join();

    // 클라이언트 서비스 중지
    service->Stop();

    return 0;
}
