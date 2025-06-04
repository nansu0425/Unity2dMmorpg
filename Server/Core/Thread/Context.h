/*    Core/Thread/Context.h    */

#pragma once

namespace core
{
    // Thread 별로 리소스를 관리하는 간소화된 ThreadContext
    class ThreadContext
    {
    public:
        ThreadContext();
        ~ThreadContext();

        // 복사 방지
        ThreadContext(const ThreadContext&) = delete;
        ThreadContext& operator=(const ThreadContext&) = delete;

        // 리소스 저장
        template<typename T>
        void set(const std::string& key, T&& value);

        // 리소스 가져오기
        template<typename T>
        T& get(const std::string& key);

        // 리소스 존재 여부 확인
        bool has(const std::string& key) const;

        // 리소스 제거
        void remove(const std::string& key);

        // 모든 리소스 정리
        void clear();

        // 현재 스레드의 소유자 ID 확인
        std::thread::id getOwnerThreadId() const { return m_ownerThreadId; }

        // 전역 애플리케이션 컨텍스트 가져오기
        static AppContext* getAppContext();

        // 전역 애플리케이션 컨텍스트 설정
        static void setAppContext(AppContext* appContext);

        // 현재 스레드의 ThreadContext 가져오기 (없으면 생성)
        static ThreadContext* getCurrent();

        // 현재 스레드의 ThreadContext 설정
        static void setCurrent(ThreadContext* context);

        // 현재 스레드의 ThreadContext 제거
        static void clearCurrent();

        // 초기화 콜백 함수 - ThreadPool별로 설정할 수 있도록 static 제거
        using InitCallback = std::function<void(ThreadContext*)>;

        // 정리 콜백 함수 - ThreadPool별로 설정할 수 있도록 static 제거
        using CleanupCallback = std::function<void(ThreadContext*)>;

        // 디버깅 지원
        static size_t getActiveContextCount();

    private:
        std::unordered_map<std::string, std::any> m_resources;
        const std::thread::id m_ownerThreadId;

        // TLS에 ThreadContext 포인터 저장
        static thread_local ThreadContext* t_currentContext;

        // 활성 컨텍스트 수 추적
        static std::atomic<size_t> s_activeContextCount;

        // 전역 애플리케이션 컨텍스트 
        static AppContext* s_appContext;
    };

} // namespace core

#include "Core/Thread/Context.inl"
