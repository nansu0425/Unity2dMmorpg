/*    Core/Thread/Context.cpp    */

#include "Core/Pch.h"
#include "Core/Thread/Context.h"

namespace core
{
    // 스레드 로컬 저장소(TLS) 변수 정의
    thread_local ThreadContext* ThreadContext::t_currentContext = nullptr;

    // 정적 변수 초기화
    std::atomic<size_t> ThreadContext::s_activeContextCount{0};
    AppContext* ThreadContext::s_appContext = nullptr;

    ThreadContext::ThreadContext()
        : m_ownerThreadId(std::this_thread::get_id())
    {}

    ThreadContext::~ThreadContext()
    {
        clear();
    }

    bool ThreadContext::has(const std::string& key) const
    {
        CORE_ASSERT(m_ownerThreadId == std::this_thread::get_id(),
                    "Attempt to check ThreadContext resource from different thread");

        return m_resources.find(key) != m_resources.end();
    }

    void ThreadContext::remove(const std::string& key)
    {
        CORE_ASSERT(m_ownerThreadId == std::this_thread::get_id(),
                    "Attempt to remove ThreadContext resource from different thread");

        auto it = m_resources.find(key);
        if (it != m_resources.end())
        {
            m_resources.erase(it);
        }
    }

    void ThreadContext::clear()
    {
        m_resources.clear();
    }

    AppContext* ThreadContext::getAppContext()
    {
        return s_appContext;
    }

    void ThreadContext::setAppContext(AppContext* appContext)
    {
        s_appContext = appContext;
    }

    ThreadContext* ThreadContext::getCurrent()
    {
        if (!t_currentContext)
        {
            t_currentContext = new ThreadContext();
            s_activeContextCount.fetch_add(1, std::memory_order_relaxed);
        }
        return t_currentContext;
    }

    void ThreadContext::setCurrent(ThreadContext* context)
    {
        if (t_currentContext != context)
        {
            // 이전 컨텍스트 정리
            clearCurrent();

            // 새 컨텍스트 설정
            t_currentContext = context;

            if (context)
            {
                s_activeContextCount.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }

    void ThreadContext::clearCurrent()
    {
        if (t_currentContext)
        {
            delete t_currentContext;
            t_currentContext = nullptr;
            s_activeContextCount.fetch_sub(1, std::memory_order_relaxed);
        }
    }

    size_t ThreadContext::getActiveContextCount()
    {
        return s_activeContextCount.load(std::memory_order_relaxed);
    }
} // namespace core
