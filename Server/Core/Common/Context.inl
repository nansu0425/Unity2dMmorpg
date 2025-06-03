/*    Core/Common/Context.inl    */

#pragma once

namespace core
{
    template<typename T, typename... Args>
    T* AppContext::registerComponent(Args&&... args)
    {
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = component.get();
        const std::string_view& name = ptr->getName();

        ComponentEntry entry;
        entry.component = std::move(component);
        entry.dependsOn = ptr->getDependencies();

        m_components[name] = std::move(entry);
        m_typeRegistry[typeid(T).name()] = ptr;

        return ptr;
    }

    template<typename T>
    T* AppContext::getComponent() const
    {
        std::string_view typeName(typeid(T).name());
        auto it = m_typeRegistry.find(typeName);
        CORE_ASSERT(it != m_typeRegistry.end(), "COMPONENT_NOT_FOUND");

        return static_cast<T*>(it->second);
    }

    template<typename T, typename... Args>
    T* AppContext::registerNamedComponent(std::string_view name, Args&&... args)
    {
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = component.get();

        ComponentEntry entry;
        entry.component = std::move(component);
        entry.dependsOn = ptr->getDependencies();

        m_components[name] = std::move(entry);

        return ptr;
    }

    template<typename T>
    T* AppContext::getNamedComponent(std::string_view name) const
    {
        auto it = m_components.find(name);
        CORE_ASSERT(it != m_components.end(), "COMPONENT_NOT_FOUND");

        return static_cast<T*>(it->second.component.get());
    }
} // namespace core
