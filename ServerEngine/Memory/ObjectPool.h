/*    ServerEngine/Memory/ObjectPool.h    */

#pragma once

#include "ServerEngine/Memory/MemoryAllocator.h"

template<typename T>
class ObjectPool
{
public:
    static void Push(T* object)
    {
        object->~T();
        ObjectPoolMemoryAllocator<T>().deallocate(object, 1);
    }

    template<typename... Args>
    static T* Pop(Args&&... args)
    {
        T* memory = ObjectPoolMemoryAllocator<T>().allocate(1);
        new (memory) T(std::forward<Args>(args)...);

        return memory;
    }
};
