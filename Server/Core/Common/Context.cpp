/*    Core/Common/Context.cpp    */

#include "Core/Pch.h"

namespace core
{
    AppContext::~AppContext()
    {
        shutdown();
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

        std::cout << "Component " << name << " has been shut down." << std::endl;
    }

    // 사이클 검출 메서드 구현
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
                    // 사이클을 발견한 후 outCyclePath 생성
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
                // 아직 방문하지 않은 의존성이면 재귀적으로 탐색
                if (!visited.count(dependency))
                {
                    if (detectCycleRec(dependency, visited, recursionStack, cycleTrace))
                    {
                        cycleTrace.insert(cycleTrace.begin(), node);
                        return true;
                    }
                }
                // 이미 현재 재귀 스택에 있는 노드라면 사이클 발견
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
} // namespace core
