/*    DummyClient/Main.cpp    */

#include "DummyClient/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Network/Service.h"
#include "DummyClient/Network/Session.h"

// TODO: 네트워크 입출력 스레드와 잡 처리 스레드를 분리할 것
void WorkerThread(SharedPtr<ClientService> service)
{
    static constexpr UInt64 kLoopTick = 64;

    while (true)
    {
        tWorkerLoopTick = ::GetTickCount64() + kLoopTick;
        // 서비스의 입출력 이벤트 전달 및 처리
        Int64 result = service->GetIoEventDispatcher()->Dispatch(10);
        // 타이머 설정 시간이 지난 잡을 큐에 분배
        gJobTimer->Distribute();
        // 아직 비우지 못한 큐를 비운다
        gJobQueueManager->FlushQueues();
    }
}

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

    // 클라이언트 서비스 생성 및 실행
    auto service = std::make_shared<ClientService>(gConfig);
    ASSERT_CRASH(SUCCESS == service->Run(), "CLIENT_SERVICE_RUN_FAILED");

    // 입출력 이벤트 처리 스레드 생성 및 실행
    for (Int64 i = 0; i < 4; ++i)
    {
        gThreadManager->Launch([service]()
                               {
                                   WorkerThread(service);
                               });
    }
    gThreadManager->Join();

    // 클라이언트 서비스 중지
    service->Stop();

    return 0;
}
