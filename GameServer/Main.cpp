/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Network/Service.h"
#include "ServerEngine/Network/Session.h"

class ClientSession
    : public Session
{
protected:
    virtual void    OnConnect() override
    {
        // 연결 성공 처리
        std::cout << "Connected to client" << std::endl;
    }

    virtual Int64   OnRecv(Byte* buffer, Int64 numBytes) override
    {
        std::cout << "Received: " << numBytes << " bytes" << std::endl;
        // 수신한 데이터를 그대로 전송
        Send(buffer, numBytes);
        return numBytes;
    }

    virtual void    OnSend(Int64 numBytes) override
    {
        // 전송 완료 처리
        std::cout << "Sent " << numBytes << " bytes" << std::endl;
    }

    virtual void    OnDisconnect() override
    {
        // 연결 종료 처리
        std::cout << "Disconnected" << std::endl;
    }
};

Service::Config gConfig =
{
    NetAddress(TEXT_16("127.0.0.1"), 7777),
    std::make_shared<IoEventDispatcher>(),
    std::make_shared<ClientSession>,
    100,
};

int main()
{
    mi_version();

    // 게임 서버 서비스 생성 및 실행
    auto gameService = std::make_shared<ServerService>(gConfig);
    ASSERT_CRASH(SUCCESS == gameService->Run(), "SERVER_SERVICE_RUN_FAILED");

    // 입출력 이벤트 처리 스레드 생성 및 실행
    for (Int64 i = 0; i < std::thread::hardware_concurrency() / 2; ++i)
    {
        gThreadManager->Launch([gameService]()
                               {
                                   Int64 result = SUCCESS;
                                   while (result == SUCCESS)
                                   {
                                       result = gameService->GetIoDispatcher()->Dispatch();
                                   }
                               });
    }
    gThreadManager->Join();

    // 게임 서버 서비스 서비스 중지
    gameService->Stop();

    return 0;
}
