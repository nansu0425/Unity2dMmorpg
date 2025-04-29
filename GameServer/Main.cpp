/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Network/Service.h"
#include "GameServer/Network/Session.h"
#include "GameServer/Network/SessionManager.h"
#include "GameServer/Network/Packet.h"
#include <io.h>
#include <fcntl.h>
#include "Packet/Generated/S_Test_generated.h"

Service::Config gConfig =
{
    NetAddress(TEXT_16("127.0.0.1"), 7777),
    std::make_shared<IoEventDispatcher>(),
    std::make_shared<GameSession>,
    1000,
};

const Byte gMessage[] = TEXT_8("Hello, World!");

int main()
{
    // 콘솔을 UTF-8 모드로 설정
    ::SetConsoleOutputCP(CP_UTF8);
    ::SetConsoleCP(CP_UTF8);
    // CRT stdout을 UTF-8 narrow-text 모드로 전환
    ::_setmode(::_fileno(stdout), _O_U8TEXT);

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
        flatbuffers::FlatBufferBuilder fbb(4096);
        // buff 1
        Int64 victimsData1[] = {4000};
        flatbuffers::Offset<flatbuffers::Vector<Int64>> victims1 = fbb.CreateVector(victimsData1, 1);
        flatbuffers::Offset<Packet::Buff> buff1 = Packet::CreateBuff(fbb, 100, 1.2f, victims1);
        // buff 2
        Int64 victimsData2[] = {1000, 2000};
        flatbuffers::Offset<flatbuffers::Vector<Int64>> victims2 = fbb.CreateVector(victimsData2, 2);
        flatbuffers::Offset<Packet::Buff> buff2 = Packet::CreateBuff(fbb, 200, 2.5f, victims2);
        // 최종 패킷 생성
        Vector<flatbuffers::Offset<Packet::Buff>> buffData = {buff1, buff2};
        auto buffs = fbb.CreateVector(buffData);
        auto packet = Packet::CreateS_Test(fbb, 1000, 100, 10, buffs);
        fbb.Finish(packet);

        gLogger->Debug(TEXT_16("Packet Size: {}"), fbb.GetSize());

        // S_Test 패킷을 모든 세션에 전송
        SharedPtr<SendBuffer> sendBuffer = ServerPacketGenerator::MakeSendBuffer(fbb, PacketId::S_Test);
        gSessionManager.Broadcast(sendBuffer);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    gThreadManager->Join();

    // 서버 서비스 서비스 중지
    service->Stop();

    return 0;
}
