/*    ServerEngine/Memory/Core.h    */

#pragma once

#include "ServerEngine/Memory/Allocator.h"

template<typename T, typename... Args>
inline T* CreateObject(Args&&... args)
{
    T* memory = static_cast<T*>(ALLOCATE_MEMORY(sizeof(T)));
    new (memory) T(std::forward<Args>(args)...);
    return memory;
}

template<typename T>
inline void DestroyObject(T* object)
{
    object->~T();
    FREE_MEMORY(object);
}
