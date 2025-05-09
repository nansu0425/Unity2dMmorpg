/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Network/Service.h"
#include "GameServer/Network/Session.h"
#include "GameServer/Network/Message.h"
#include "Common/Message.h"
#include "GameServer/Content/Room.h"
#include "ServerEngine/Database/Connection.h"

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
    ASSERT_CRASH(gDbConnectionPool->Connect(1, TEXT_16("Driver={ODBC Driver 17 for SQL Server};Server=(localdb)\\MSSQLLocalDB;Database=GameServerDb;Trusted_Connection=Yes;")));

    // Create table
    {
        String16View query = TEXT_16("                                  \
                            DROP TABLE IF EXISTS [dbo].[Gold];          \
                            CREATE TABLE [dbo].[Gold]                   \
                            (                                           \
                                [id] INT NOT NULL PRIMARY KEY IDENTITY, \
                                [gold] INT NULL                         \
                            );");
        SharedPtr<DbConnection> connection = gDbConnectionPool->Pop();
        ASSERT_CRASH(connection->Execute(query));
        gDbConnectionPool->Push(connection);
    }

    // Add data
    for (Int64 i = 0; i < 3; ++i)
    {
        SharedPtr<DbConnection> connection = gDbConnectionPool->Pop();
        connection->Unbind();

        Int32 gold = 100;
        SQLLEN len = 0;

        // 파라미터 바인딩
        ASSERT_CRASH(connection->BindParameter(1, SQL_C_LONG, SQL_INTEGER, SIZE_32(gold), &gold, OUT &len));
        // Insert 쿼리
        ASSERT_CRASH(connection->Execute(TEXT_16("INSERT INTO [dbo].[Gold]([gold]) VALUES(?)")));
        gDbConnectionPool->Push(connection);
    }

    // Read data
    {
        SharedPtr<DbConnection> connection = gDbConnectionPool->Pop();
        connection->Unbind();
        Int32 gold = 100;
        SQLLEN len = 0;
        // 파라미터 바인딩
        ASSERT_CRASH(connection->BindParameter(1, SQL_C_LONG, SQL_INTEGER, SIZE_32(gold), &gold, OUT & len));

        Int32 outId = 0;
        SQLLEN outIdLen = 0;
        ASSERT_CRASH(connection->BindColumns(1, SQL_C_LONG, OUT & outId, SIZE_32(outId), OUT & outIdLen));

        Int32 outGold = 0;
        SQLLEN outGoldLen = 0;
        ASSERT_CRASH(connection->BindColumns(2, SQL_C_LONG, OUT & outGold, SIZE_32(outGold), OUT & outGoldLen));

        // Select 쿼리
        ASSERT_CRASH(connection->Execute(TEXT_16("SELECT id, gold FROM [dbo].[Gold] WHERE gold = (?)")));

        // Fetch
        while (connection->Fetch())
        {
            gLogger->Info(TEXT_16("id: {}, gold: {}"), outId, outGold);
        }

        gDbConnectionPool->Push(connection);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
