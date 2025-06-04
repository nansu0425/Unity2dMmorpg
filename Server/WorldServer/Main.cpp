/*    WorldServer/Main.cpp    */

#include "WorldServer/Pch.h"
#include "Core/Thread/Pool.h"
#include "Core/Thread/Context.h"

// 네트워크 작업을 시뮬레이션하는 워커 루틴
void networkWorkerRoutine(core::ThreadContext* ctx, size_t threadIndex, std::atomic<bool>& shutdown)
{
    std::cout << "네트워크 워커 #" << threadIndex << " 시작됨" << std::endl;

    int counter = 0;
    while (!shutdown.load())
    {
        // 네트워크 작업 시뮬레이션
        std::cout << "네트워크 워커 #" << threadIndex
            << " 작업 중... (카운터: " << counter << ")" << std::endl;

        counter++;

        // 일부러 지연 추가 (실제 네트워크 작업 시뮬레이션)
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 테스트를 위해 반복 후 종료
        if (counter >= 5)
        {
            std::cout << "네트워크 워커 #" << threadIndex << " 작업 완료" << std::endl;
            break;
        }
    }

    std::cout << "네트워크 워커 #" << threadIndex << " 종료됨" << std::endl;
}

// 데이터베이스 작업을 시뮬레이션하는 워커 루틴
void databaseWorkerRoutine(core::ThreadContext* ctx, size_t threadIndex, std::atomic<bool>& shutdown)
{
    std::cout << "데이터베이스 워커 #" << threadIndex << " 시작됨" << std::endl;

    // 상태 정보를 ThreadContext에 저장
    ctx->set("connectionState", std::string("Connected"));

    int queryCount = 0;
    while (!shutdown.load())
    {
        // DB 작업 시뮬레이션
        queryCount++;
        std::cout << "데이터베이스 워커 #" << threadIndex
            << " 쿼리 실행 중... (총 " << queryCount << "개 쿼리 실행)" << std::endl;

        // 현재 연결 상태 확인 (ThreadContext에서 값 가져오기)
        std::string state = ctx->get<std::string>("connectionState");
        std::cout << "데이터베이스 연결 상태: " << state << std::endl;

        // 일부러 지연 추가
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));

        // 테스트를 위해 반복 후 종료
        if (queryCount >= 3)
        {
            ctx->set("connectionState", std::string("Disconnected"));
            std::cout << "데이터베이스 워커 #" << threadIndex << " 작업 완료" << std::endl;
            break;
        }
    }

    std::cout << "데이터베이스 워커 #" << threadIndex << " 종료됨" << std::endl;
}

// AppContext에 등록할 테스트용 컴포넌트
class TestComponent : public core::IAppComponent
{
public:
    bool initialize() override
    {
        std::cout << "TestComponent 초기화됨" << std::endl;
        return true;
    }

    void shutdown() override
    {
        std::cout << "TestComponent 종료됨" << std::endl;
    }

    std::string_view getName() const override
    {
        return "TestComponent";
    }

    void performTask()
    {
        std::cout << "TestComponent 작업 수행 중..." << std::endl;
    }
};

int main()
{
    std::cout << "===== 컴포넌트 시스템 테스트 시작 =====" << std::endl;

    // 1. AppContext 초기화
    core::AppContext appContext;

    // 테스트용 컴포넌트 등록
    auto* testComponent = appContext.registerComponent<TestComponent>();

    // ThreadPoolManager 등록
    auto* poolManager = appContext.registerComponent<core::ThreadPoolManager>();

    // AppContext 초기화 (이 과정에서 모든 컴포넌트 초기화)
    appContext.initialize();

    // 컴포넌트 사용 테스트
    testComponent->performTask();

    std::cout << "\n===== ThreadPool 및 ThreadContext 테스트 =====" << std::endl;

    // 2. 네트워크 스레드 풀 생성 및 설정
    auto* networkPool = poolManager->createThreadPool("Network");

    // 네트워크 스레드 초기화 콜백 설정
    networkPool->setThreadInitCallback([](core::ThreadContext* ctx)
                                       {
                                           std::cout << "네트워크 스레드 초기화 중..." << std::endl;
                                           ctx->set("type", std::string("Network"));
                                           ctx->set("priority", std::string("High"));
                                       });

    // 네트워크 스레드 정리 콜백 설정
    networkPool->setThreadCleanupCallback([](core::ThreadContext* ctx)
                                          {
                                              std::cout << "네트워크 스레드 정리 중... (풀: "
                                                  << ctx->get<std::string>("poolName")
                                                  << ", 인덱스: " << ctx->get<size_t>("threadIndex") << ")" << std::endl;
                                          });

    // 네트워크 워커 루틴 설정
    networkPool->setWorkerRoutine(networkWorkerRoutine);

    std::cout << "\n네트워크 스레드 풀 시작 (2개 스레드)" << std::endl;
    networkPool->startWorkerThreads(2);

    // 3. 데이터베이스 스레드 풀 생성 및 설정
    auto* dbPool = poolManager->createThreadPool("Database");

    // 데이터베이스 스레드 초기화 콜백 설정
    dbPool->setThreadInitCallback([](core::ThreadContext* ctx)
                                  {
                                      std::cout << "데이터베이스 스레드 초기화 중..." << std::endl;
                                      ctx->set("type", std::string("Database"));
                                      ctx->set("connectionString", std::string("Server=localhost;Database=testdb"));
                                  });

    // 데이터베이스 워커 루틴 설정
    dbPool->setWorkerRoutine(databaseWorkerRoutine);

    std::cout << "\n데이터베이스 스레드 풀 시작 (1개 스레드)" << std::endl;
    dbPool->startWorkerThreads(1);

    // 4. 전역 AppContext 및 현재 스레드의 ThreadContext 테스트
    std::cout << "\n===== 글로벌 헬퍼 함수 테스트 =====" << std::endl;

    // 메인 스레드에서 ThreadContext 사용
    auto* mainThreadCtx = core::GetThreadContext();
    mainThreadCtx->set("name", std::string("MainThread"));
    mainThreadCtx->set("startTime", std::chrono::system_clock::now());

    std::cout << "메인 스레드 컨텍스트 이름: " << mainThreadCtx->get<std::string>("name") << std::endl;

    // AppContext 접근
    auto* globalAppCtx = core::GetAppContext();
    std::cout << "글로벌 AppContext 접근 테스트: "
        << (globalAppCtx == &appContext ? "성공" : "실패") << std::endl;

    // 5. 스레드 풀 관리 기능 테스트
    std::cout << "\n===== ThreadPoolManager 기능 테스트 =====" << std::endl;

    // 존재하는 스레드 풀 가져오기
    auto* existingPool = poolManager->getThreadPool("Network");
    std::cout << "기존 스레드 풀 조회: " << existingPool->getName() << std::endl;

    // 기본 스레드 풀 가져오기
    auto* defaultPool = poolManager->getDefaultThreadPool();
    std::cout << "기본 스레드 풀 조회: " << defaultPool->getName() << std::endl;

    // 6. 일정 시간 동안 실행 후 정리
    std::cout << "\n모든 스레드 풀 실행 중... (10초 후 종료)" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "\n===== 정리 단계 =====" << std::endl;

    // 개별 스레드 풀 종료
    std::cout << "네트워크 스레드 풀 종료 중..." << std::endl;
    networkPool->joinWorkerThreads();

    std::cout << "데이터베이스 스레드 풀 종료 중..." << std::endl;
    dbPool->joinWorkerThreads();

    // AppContext 종료 (이 과정에서 모든 컴포넌트 종료)
    std::cout << "\nAppContext 종료 중..." << std::endl;
    appContext.shutdown();

    std::cout << "\n===== 테스트 완료 =====" << std::endl;

    return 0;
}
