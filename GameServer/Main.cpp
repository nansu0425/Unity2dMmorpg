/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Network/Service.h"
#include "GameServer/Network/Session.h"
#include "GameServer/Network/Message.h"
#include "Common/Message.h"
#include "GameServer/Content/Room.h"

#include <io.h>
#include <fcntl.h>

Service::Config gConfig =
{
    NetAddress(TEXT_16("127.0.0.1"), 7777),
    std::make_shared<IoEventDispatcher>(),
    std::make_shared<ClientSession>,
    100,
};

void WorkerThread(SharedPtr<ServerService> service)
{
    static constexpr UInt64 kLoopTick = 64;

    while (true)
    {
        tWorkerLoopTick = ::GetTickCount64() + kLoopTick;
        // 네트워크 입출력부터 콘텐츠 로직까지 처리
        Int64 result = service->GetIoDispatcher()->Dispatch(10);
        // 타이머에 의해 스케줄링된 작업들을 분배
        gJobTimer->Distribute();
        // 아직 비우지 못한 큐들을 비운다
        gJobQueueManager->FlushQueues();
    }
}

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

    // 워커 스레드 실행
    for (Int64 i = 0; i < std::thread::hardware_concurrency() / 2; ++i)
    {
        gThreadManager->Launch([service]()
                               {
                                   WorkerThread(service);
                               });
    }
    WorkerThread(service);

    gThreadManager->Join();

    // 서버 서비스 서비스 중지
    service->Stop();

    return 0;
}
