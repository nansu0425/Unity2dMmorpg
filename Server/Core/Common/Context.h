/*    Core/Common/Context.h    */

#pragma once

namespace core
{
    class IAppComponent
    {
    public:
        virtual ~IAppComponent() = default;
        virtual bool initialize() = 0;
        virtual void shutdown() = 0;
        virtual std::string_view getName() const = 0;

        // 선택적 의존성 관리
        virtual std::vector<std::string_view> getDependencies() const { return {}; }
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
        T* registerComponent(Args&&... args);

        // 컴포넌트 검색
        template<typename T>
        T* getComponent() const;

        // 이름이 지정된 컴포넌트 등록
        template<typename T, typename... Args>
        T* registerNamedComponent(std::string_view name, Args&&... args);

        // 이름이 지정된 컴포넌트 검색
        template<typename T>
        T* getNamedComponent(std::string_view name) const;

        // 의존성 순서에 따라 모든 컴포넌트 초기화
        void initialize();

        // 역순으로 모든 컴포넌트 종료
        void shutdown();

    private:
        void initializeComponent(std::string_view name);
        void shutdownComponent(std::string_view name);

        // 사이클 탐지 관련 메서드
        bool detectCycle(std::string* outCyclePath = nullptr);
        bool detectCycleRec(std::string_view node, std::unordered_set<std::string_view>& visited,
                            std::unordered_set<std::string_view>& recursionStack,
                            std::vector<std::string_view>& cycleTrace);

        struct ComponentEntry
        {
            std::unique_ptr<IAppComponent> component;
            bool initialized = false;
            std::vector<std::string_view> dependsOn;
        };

        std::unordered_map<std::string_view, ComponentEntry> m_components;
        std::unordered_map<std::string_view, IAppComponent*> m_typeRegistry;
        std::vector<std::string_view> m_initOrder; // 초기화 순서를 추적하여 올바른 종료 순서 보장
    };
} // namespace core

#include "Core/Common/Context.inl"
