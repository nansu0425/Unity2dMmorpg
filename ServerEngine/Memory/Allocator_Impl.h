#include "Allocator.h"
#pragma once

template<typename T, typename... Args>
inline T* CreateObject(Args&&... args)
{
    T* memory = static_cast<T*>(::AllocateMemory(sizeof(T)));
    new (memory) T(std::forward<Args>(args)...);
    return memory;
}

template<typename T>
inline void DestroyObject(T* object)
{
    object->~T();
    ::FreeMemory(object);
}
