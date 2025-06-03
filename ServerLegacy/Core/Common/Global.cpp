/*    Core/Common/Global.cpp    */

#include "Core/Pch.h"
#include "Core/Log/Logger.h"
#include "Core/Concurrency/Thread.h"
#include "Core/Concurrency/Deadlock.h"
#include "Core/Network/Socket.h"

namespace core
{
    Logger* gLogger = nullptr;
    ThreadManager* gThreadManager = nullptr;
    DeadlockDetector* gDeadlockDetector = nullptr;
    SendChunkPool* gSendChunkPool = nullptr;
    JobQueueManager* gJobQueueManager = nullptr;
    JobTimer* gJobTimer = nullptr;

    GlobalContext::GlobalContext()
    {
        gLogger = new Logger(TEXT_8("GlobalLogger"));
        gThreadManager = new ThreadManager();
        gDeadlockDetector = new DeadlockDetector();
        gSendChunkPool = new SendChunkPool();
        SocketUtils::Init();
        gJobQueueManager = new JobQueueManager();
        gJobTimer = new JobTimer();
    }

    GlobalContext::~GlobalContext()
    {
        delete gJobTimer;
        delete gJobQueueManager;
        SocketUtils::Cleanup();
        delete gSendChunkPool;
        delete gDeadlockDetector;
        delete gThreadManager;
        delete gLogger;
    }

    GlobalContext   gGlobalContext;

    AppContext::~AppContext()
    {
        Shutdown();
    }

    void AppContext::Initialize()
    {
        m_initOrder.clear();

        // 사이클 검출 결과를 저장할 변수
        String8 cyclePath;
        ASSERT_CRASH_DEBUG(!DetectCycle(&cyclePath), "CYCLE_DETECTED");

        // 의존성을 고려하여 모든 컴포넌트 초기화
        for (const auto& [name, entry] : m_components)
        {
            InitializeComponent(name);
        }
    }

    void AppContext::Shutdown()
    {
        // 초기화의 역순으로 종료
        for (auto it = m_initOrder.rbegin(); it != m_initOrder.rend(); ++it)
        {
            ShutdownComponent(*it);
        }
        m_initOrder.clear();
    }

    void AppContext::InitializeComponent(String8View name)
    {
        auto it = m_components.find(name);
        ASSERT_CRASH_DEBUG(it != m_components.end(), "COMPONENT_NOT_FOUND");

        auto& entry = it->second;

        // 이미 초기화된 경우
        if (entry.initialized)
        {
            return;
        }

        // 먼저 의존성 컴포넌트 초기화
        for (const auto& dependency : entry.dependsOn)
        {
            InitializeComponent(dependency);
        }

        // 컴포넌트 초기화
        Bool initialized = entry.component->Initialize();
        ASSERT_CRASH_DEBUG(initialized, "COMPONENT_INITIALIZE_FAILED");

        entry.initialized = true;
        m_initOrder.push_back(name);
    }

    void AppContext::ShutdownComponent(String8View name)
    {
        auto it = m_components.find(name);
        ASSERT_CRASH_DEBUG(it != m_components.end(), "COMPONENT_NOT_FOUND");

        // 이미 종료된 경우
        if (!it->second.initialized)
        {
            return;
        }

        it->second.component->Shutdown();
        it->second.initialized = false;

        std::cout << "Component " << name << " has been shut down." << std::endl;
    }

    // 사이클 검출 메서드 구현
    Bool AppContext::DetectCycle(String8* outCyclePath)
    {
        HashSet<String8View> visited;
        HashSet<String8View> recursionStack;
        Vector<String8View> cycleTrace;

        for (const auto& [name, _] : m_components)
        {
            if (!visited.count(name))
            {
                if (DetectCycleRec(name, visited, recursionStack, cycleTrace))
                {
                    // 사이클을 발견한 후 outCyclePath 생성
                    if (outCyclePath && !cycleTrace.empty())
                    {
                        *outCyclePath = "";
                        Bool first = true;
                        for (const auto& node : cycleTrace)
                        {
                            if (!first)
                            {
                                *outCyclePath += " -> ";
                            }
                            *outCyclePath += String8(node);
                            first = false;
                        }
                    }
                    return true;
                }
            }
        }

        return false;
    }

    Bool AppContext::DetectCycleRec(String8View node, HashSet<String8View>& visited, HashSet<String8View>& recursionStack, Vector<String8View>& cycleTrace)
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
                    if (DetectCycleRec(dependency, visited, recursionStack, cycleTrace))
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
