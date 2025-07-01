#pragma once

namespace core
{
    template<typename T, typename... Args>
    T* AppContext::RegisterComponent(Args&&... args)
    {
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = component.get();
        const String8View& name = ptr->GetName();

        ComponentEntry entry;
        entry.component = std::move(component);
        entry.dependsOn = ptr->GetDependencies();

        m_components[name] = std::move(entry);
        m_typeRegistry[typeid(T).name()] = ptr;

        return ptr;
    }

    template<typename T>
    T* AppContext::GetComponent() const
    {
        String8View typeName(typeid(T).name());
        auto it = m_typeRegistry.find(typeName);
        return it != m_typeRegistry.end() ? static_cast<T*>(it->second) : nullptr;
    }

    template<typename T, typename... Args>
    T* AppContext::RegisterNamedComponent(String8View name, Args&&... args)
    {
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = component.get();

        ComponentEntry entry;
        entry.component = std::move(component);
        entry.dependsOn = ptr->GetDependencies();

        m_components[name] = std::move(entry);

        return ptr;
    }

    template<typename T>
    T* AppContext::GetNamedComponent(String8View name) const
    {
        auto it = m_components.find(name);
        return it != m_components.end() ? static_cast<T*>(it->second.component.get()) : nullptr;
    }
} // namespace core
