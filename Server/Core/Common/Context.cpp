/*    Core/Common/Context.cpp    */

#include "Core/Pch.h"
#include "Core/Thread/Context.h"

namespace core
{
    // 싱글턴 인스턴스 초기화
    AppContext* AppContext::s_instance = nullptr;

    AppContext::AppContext()
    {
        // 싱글턴 인스턴스 설정
        if (!s_instance)
        {
            s_instance = this;

            // ThreadContext에 AppContext 참조 설정
            ThreadContext::setAppContext(this);
        }
    }

    AppContext::~AppContext()
    {
        shutdown();

        if (s_instance == this)
        {
            s_instance = nullptr;

            // 애플리케이션 종료 시 ThreadContext 설정 지우기
            ThreadContext::setAppContext(nullptr);
        }
    }

    void AppContext::initialize()
    {
        m_initOrder.clear();

        // 사이클 검출 결과를 저장할 변수
        std::string cyclePath;
        CORE_ASSERT(!detectCycle(&cyclePath), std::string("CYCLE_DETECTED: ") + cyclePath);

        // 의존성을 고려하여 모든 컴포넌트 초기화
        for (const auto& [name, entry] : m_components)
        {
            initializeComponent(name);
        }
    }

    void AppContext::shutdown()
    {
        // 초기화의 역순으로 종료
        for (auto it = m_initOrder.rbegin(); it != m_initOrder.rend(); ++it)
        {
            shutdownComponent(*it);
        }
        m_initOrder.clear();
    }

    void AppContext::initializeComponent(std::string_view name)
    {
        auto it = m_components.find(name);
        CORE_ASSERT(it != m_components.end(), "COMPONENT_NOT_FOUND");

        auto& entry = it->second;

        // 이미 초기화된 경우
        if (entry.initialized)
        {
            return;
        }

        // 먼저 의존성 컴포넌트 초기화
        for (const auto& dependency : entry.dependsOn)
        {
            initializeComponent(dependency);
        }

        // 컴포넌트 초기화
        bool initialized = entry.component->initialize();
        CORE_ASSERT(initialized, "COMPONENT_INITIALIZE_FAILED");

        entry.initialized = true;
        m_initOrder.push_back(name);
    }

    void AppContext::shutdownComponent(std::string_view name)
    {
        auto it = m_components.find(name);
        CORE_ASSERT(it != m_components.end(), "COMPONENT_NOT_FOUND");

        // 이미 종료된 경우
        if (!it->second.initialized)
        {
            return;
        }

        it->second.component->shutdown();
        it->second.initialized = false;
    }

    // 사이클 검출 메서드 구현은 기존과 동일하게 유지

    bool AppContext::detectCycle(std::string* outCyclePath)
    {
        std::unordered_set<std::string_view> visited;
        std::unordered_set<std::string_view> recursionStack;
        std::vector<std::string_view> cycleTrace;

        for (const auto& [name, _] : m_components)
        {
            if (!visited.count(name))
            {
                if (detectCycleRec(name, visited, recursionStack, cycleTrace))
                {
                    if (outCyclePath && !cycleTrace.empty())
                    {
                        *outCyclePath = "";
                        bool first = true;
                        for (const auto& node : cycleTrace)
                        {
                            if (!first)
                            {
                                *outCyclePath += " -> ";
                            }
                            *outCyclePath += std::string(node);
                            first = false;
                        }
                    }
                    return true;
                }
            }
        }

        return false;
    }

    bool AppContext::detectCycleRec(std::string_view node, std::unordered_set<std::string_view>& visited,
                                    std::unordered_set<std::string_view>& recursionStack,
                                    std::vector<std::string_view>& cycleTrace)
    {
        visited.insert(node);
        recursionStack.insert(node);

        auto it = m_components.find(node);
        if (it != m_components.end())
        {
            for (const auto& dependency : it->second.dependsOn)
            {
                if (!visited.count(dependency))
                {
                    if (detectCycleRec(dependency, visited, recursionStack, cycleTrace))
                    {
                        cycleTrace.insert(cycleTrace.begin(), node);
                        return true;
                    }
                }
                else if (recursionStack.count(dependency))
                {
                    cycleTrace.push_back(dependency);
                    cycleTrace.insert(cycleTrace.begin(), node);
                    return true;
                }
            }
        }

        recursionStack.erase(node);
        return false;
    }

    AppContext* AppContext::getInstance()
    {
        return s_instance;
    }

    // 글로벌 헬퍼 함수
    AppContext* GetAppContext()
    {
        return ThreadContext::getAppContext();
    }

    ThreadContext* GetThreadContext()
    {
        return ThreadContext::getCurrent();
    }

} // namespace core
