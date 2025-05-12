/*    DummyClient/Main.cpp    */

#include "DummyClient/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Network/Service.h"
#include "DummyClient/Network/Session.h"
#include "GameContent/Chat/Room.h"

constexpr Byte gBroadcastMessage[] = TEXT_8("Hello, world!");

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

void BroadcastAsync(SharedPtr<SendBuffer> sendBuf)
{
    // 1초마다 브로드캐스트
    gRoom->MakeJob([sendBuf]
                   {
                       gRoom->Broadcast(sendBuf);
                       BroadcastAsync(sendBuf);
                   }, 1000);

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

    // 브로드캐스트 메시지를 송신 버퍼로 만든다
    SharedPtr<SendBuffer> sendBuf = gSendChunkPool->Alloc(1024);
    BufferWriter writer(sendBuf->GetBuffer(), sendBuf->GetAllocSize());
    writer.Write(gBroadcastMessage, NUM_ELEM_64(gBroadcastMessage));
    sendBuf->OnWritten(NUM_ELEM_64(gBroadcastMessage));

    // 브로드캐스트 메시지를 모든 서버 세션에 1초마다 전송
    BroadcastAsync(std::move(sendBuf));

    gThreadManager->Join();

    // 클라이언트 서비스 중지
    service->Stop();

    return 0;
}
