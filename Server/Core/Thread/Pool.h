/*    Core/Thread/Pool.h    */

#pragma once

namespace core
{
    class ThreadContext;

    // 스레드 풀 관리를 위한 클래스
    class ThreadPool
    {
    public:
        ThreadPool(const std::string& name);
        ~ThreadPool();

        // 복사 방지
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        // 워커 스레드 루틴 설정 (외부에서 주입)
        using WorkerRoutine = std::function<void(ThreadContext*, size_t threadIndex, std::atomic<bool>& shutdown)>;
        void setWorkerRoutine(WorkerRoutine routine);

        // 스레드 컨텍스트 초기화 콜백 설정
        using ThreadInitCallback = std::function<void(ThreadContext*)>;
        void setThreadInitCallback(ThreadInitCallback callback);

        // 스레드 컨텍스트 정리 콜백 설정
        using ThreadCleanupCallback = std::function<void(ThreadContext*)>;
        void setThreadCleanupCallback(ThreadCleanupCallback callback);

        // 워커 스레드 시작
        void startWorkerThreads(size_t numThreads);

        // 워커 스레드 종료 대기
        void joinWorkerThreads();

        // 종료 중인지 확인
        bool isShuttingDown() const { return m_shutdown.load(); }

        // 이름 가져오기
        const std::string& getName() const { return m_name; }

        // 실행 중인 스레드 수 가져오기
        size_t getNumThreads() const { return m_threads.size(); }

    private:
        // 워커 스레드 래퍼 함수
        void workerThread(size_t threadIndex);

        std::string m_name;
        std::vector<std::thread> m_threads;
        std::atomic<bool> m_shutdown{false};

        ThreadInitCallback m_threadInitCallback;
        ThreadCleanupCallback m_threadCleanupCallback;
        WorkerRoutine m_workerRoutine;
    };

    // 애플리케이션의 모든 스레드 풀 관리
    class ThreadPoolManager : public IAppComponent
    {
    public:
        ThreadPoolManager();
        ~ThreadPoolManager() override;

        // IAppComponent 인터페이스 구현
        bool initialize() override;
        void shutdown() override;
        std::string_view getName() const override { return "ThreadPoolManager"; }

        // 스레드 풀 생성 및 관리
        ThreadPool* createThreadPool(const std::string& name);
        ThreadPool* getThreadPool(const std::string& name) const;
        void removeThreadPool(const std::string& name);

        // 기본 스레드 풀 접근
        ThreadPool* getDefaultThreadPool() const { return getThreadPool("Default"); }

    private:
        std::unordered_map<std::string, std::unique_ptr<ThreadPool>> m_threadPools;
        mutable std::mutex m_poolsMutex;
    };

} // namespace core
