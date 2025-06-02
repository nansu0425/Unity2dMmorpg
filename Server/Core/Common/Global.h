/*    Core/Common/Global.h    */

#pragma once

namespace core
{
    extern class Logger* gLogger;
    extern class ThreadManager* gThreadManager;
    extern class DeadlockDetector* gDeadlockDetector;
    extern class SendChunkPool* gSendChunkPool;
    extern class JobQueueManager* gJobQueueManager;
    extern class JobTimer* gJobTimer;

    class GlobalContext
    {
    public:
        GlobalContext();
        ~GlobalContext();
    };

    extern GlobalContext    gGlobalContext;

    class IAppComponent
    {
    public:
        virtual                         ~IAppComponent() = default;
        virtual Bool                    Initialize() = 0;
        virtual void                    Shutdown() = 0;
        virtual String8View             GetName() const = 0;

        // 선택적 의존성 관리
        virtual Vector<String8View>     GetDependencies() const { return {}; }
    };

    class AppContext
    {
    public:
        AppContext() = default;
        ~AppContext();

        // 복사 방지
        AppContext(const AppContext&) = delete;
        AppContext& operator=(const AppContext&) = delete;

        // 컴포넌트 등록
        template<typename T, typename... Args>
        T* RegisterComponent(Args&&... args);

        // 컴포넌트 검색
        template<typename T>
        T* GetComponent() const;

        // 이름이 지정된 컴포넌트 등록
        template<typename T, typename... Args>
        T* RegisterNamedComponent(String8View name, Args&&... args);

        // 이름이 지정된 컴포넌트 검색
        template<typename T>
        T* GetNamedComponent(String8View name) const;

        // 의존성 순서에 따라 모든 컴포넌트 초기화
        void Initialize();

        // 역순으로 모든 컴포넌트 종료
        void Shutdown();

    private:
        void InitializeComponent(String8View name);
        void ShutdownComponent(String8View name);

        // 사이클 탐지 관련 메서드
        Bool DetectCycle(String8* outCyclePath = nullptr);
        Bool DetectCycleRec(String8View node, HashSet<String8View>& visited, HashSet<String8View>& recursionStack, Vector<String8View>& cycleTrace);

        struct ComponentEntry
        {
            UniquePtr<IAppComponent>    component;
            Bool                        initialized = false;
            Vector<String8View>         dependsOn;
        };

        HashMap<String8View, ComponentEntry>    m_components;
        HashMap<String8View, IAppComponent*>    m_typeRegistry;
        Vector<String8View>                     m_initOrder; // 초기화 순서를 추적하여 올바른 종료 순서 보장
    };
} // namespace core

#include "Core/Common/Global.inl"
