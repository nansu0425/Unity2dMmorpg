/*    Core/Thread/Pool.cpp    */

#include "Core/Pch.h"
#include "Core/Thread/Pool.h"
#include "Core/Thread/Context.h"

namespace core
{
    // ThreadPool 구현
    ThreadPool::ThreadPool(const std::string& name)
        : m_name(name)
    {}

    ThreadPool::~ThreadPool()
    {
        joinWorkerThreads();
    }

    void ThreadPool::setWorkerRoutine(WorkerRoutine routine)
    {
        m_workerRoutine = std::move(routine);
    }

    void ThreadPool::setThreadInitCallback(ThreadInitCallback callback)
    {
        m_threadInitCallback = std::move(callback);
    }

    void ThreadPool::setThreadCleanupCallback(ThreadCleanupCallback callback)
    {
        m_threadCleanupCallback = std::move(callback);
    }

    void ThreadPool::startWorkerThreads(size_t numThreads)
    {
        // 워커 루틴이 설정되어 있는지 확인
        CORE_ASSERT(m_workerRoutine, "Worker routine not set for thread pool");

        // 이미 실행 중이면 먼저 종료
        if (!m_threads.empty())
        {
            joinWorkerThreads();
        }

        m_shutdown.store(false);

        // 워커 스레드 생성
        for (size_t i = 0; i < numThreads; ++i)
        {
            m_threads.emplace_back(&ThreadPool::workerThread, this, i);
        }
    }

    void ThreadPool::joinWorkerThreads()
    {
        m_shutdown.store(true);

        // 모든 스레드 조인
        for (auto& thread : m_threads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }

        m_threads.clear();
    }

    void ThreadPool::workerThread(size_t threadIndex)
    {
        // ThreadContext 생성 및 설정
        ThreadContext* ctx = ThreadContext::getCurrent();

        // 스레드 컨텍스트에 기본 정보 설정
        ctx->set("poolName", m_name);
        ctx->set("threadIndex", threadIndex);

        // 스레드 초기화 콜백 실행 (설정된 경우)
        if (m_threadInitCallback)
        {
            m_threadInitCallback(ctx);
        }

        try
        {
            // 외부에서 주입된 워커 루틴 실행
            if (m_workerRoutine)
            {
                m_workerRoutine(ctx, threadIndex, m_shutdown);
            }
        }
        catch (const std::exception& e)
        {
            // 예외 처리 - 로그 남기기
            std::cerr << "Exception in thread pool '" << m_name << "', thread "
                << threadIndex << ": " << e.what() << std::endl;
        }
        catch (...)
        {
            // 알 수 없는 예외 처리
            std::cerr << "Unknown exception in thread pool '" << m_name << "', thread "
                << threadIndex << std::endl;
        }

        // 스레드 정리 콜백 실행 (설정된 경우)
        if (m_threadCleanupCallback)
        {
            m_threadCleanupCallback(ctx);
        }

        // 스레드 컨텍스트 정리
        ThreadContext::clearCurrent();
    }

    // ThreadPoolManager 구현
    ThreadPoolManager::ThreadPoolManager() = default;

    ThreadPoolManager::~ThreadPoolManager()
    {
        shutdown();
    }

    bool ThreadPoolManager::initialize()
    {
        // 기본 스레드 풀은 생성만 하고 WorkerRoutine과 스레드 수는 
        // 외부에서 설정하도록 함
        createThreadPool("Default");
        return true;
    }

    void ThreadPoolManager::shutdown()
    {
        std::lock_guard<std::mutex> lock(m_poolsMutex);

        // 모든 스레드 풀 종료
        m_threadPools.clear();
    }

    ThreadPool* ThreadPoolManager::createThreadPool(const std::string& name)
    {
        CORE_ASSERT(!name.empty(), "Thread pool name cannot be empty");

        std::lock_guard<std::mutex> lock(m_poolsMutex);

        // 이미 존재하는 스레드 풀이 있는지 확인
        auto it = m_threadPools.find(name);
        if (it != m_threadPools.end())
        {
            return it->second.get();
        }

        // 새 스레드 풀 생성
        auto threadPool = std::make_unique<ThreadPool>(name);
        ThreadPool* result = threadPool.get();

        m_threadPools[name] = std::move(threadPool);
        return result;
    }

    ThreadPool* ThreadPoolManager::getThreadPool(const std::string& name) const
    {
        std::lock_guard<std::mutex> lock(m_poolsMutex);

        auto it = m_threadPools.find(name);
        CORE_ASSERT(it != m_threadPools.end(), "Thread pool not found: " + name);

        return it->second.get();
    }

    void ThreadPoolManager::removeThreadPool(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(m_poolsMutex);

        // 기본 스레드 풀은 제거할 수 없음
        CORE_ASSERT(name != "Default", "Cannot remove the default thread pool");

        auto it = m_threadPools.find(name);
        CORE_ASSERT(it != m_threadPools.end(), "Thread pool not found for removal: " + name);

        m_threadPools.erase(it);
    }

} // namespace core
