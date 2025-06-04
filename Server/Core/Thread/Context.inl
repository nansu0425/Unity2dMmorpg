/*    Core/Thread/Context.inl    */

#pragma once

namespace core
{
    template<typename T>
    void ThreadContext::set(const std::string& key, T&& value)
    {
        // 스레드 소유권 검증
        CORE_ASSERT(m_ownerThreadId == std::this_thread::get_id(),
                    "Attempt to set ThreadContext resource from different thread");

        m_resources[key] = std::forward<T>(value);
    }

    template<typename T>
    T& ThreadContext::get(const std::string& key)
    {
        // 스레드 소유권 검증
        CORE_ASSERT(m_ownerThreadId == std::this_thread::get_id(),
                    "Attempt to access ThreadContext resource from different thread");

        // 키 존재 여부 확인
        auto it = m_resources.find(key);
        CORE_ASSERT(it != m_resources.end(), "Resource not found in ThreadContext");

        try
        {
            return std::any_cast<T&>(it->second);
        }
        catch (const std::bad_any_cast&)
        {
            CORE_ASSERT(false, "Type mismatch in ThreadContext resource");
            // 예외 발생 시 프로그램이 중단되지 않도록 정적 기본값 반환
            // 릴리즈 모드에서만 도달할 수 있음
            static T defaultValue{};
            return defaultValue;
        }
    }
}
