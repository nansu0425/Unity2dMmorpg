/*    ServerEngine/Memory/Core.h    */

#pragma once

#include "ServerEngine/Memory/Allocator.h"

template<typename T, typename... Args>
inline T* NewObject(Args&&... args)
{
    T* memory = static_cast<T*>(ALLOC_MEMORY(sizeof(T)));
    new (memory) T(std::forward<Args>(args)...);
    return memory;
}

template<typename T>
inline void DeleteObject(T* object)
{
    object->~T();
    FREE_MEMORY(object);
}
