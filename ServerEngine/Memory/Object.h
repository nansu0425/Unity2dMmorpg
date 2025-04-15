/*    ServerEngine/Memory/Object.h    */

#pragma once

#include "ServerEngine/Memory/Allocator.h"
#include "ServerEngine/Memory/MemoryPool.h"

/*
 * 클래스 별로 별도의 메모리 풀을 갖게 된다
 * 오브젝트의 메모리는 메모리 풀에 의해 관리된다
 */
template<typename T>
class ObjectPool
{
public:
    template<typename... Args>
    static T* Pop(Args&&... args)
    {
        T* memory = static_cast<T*>(MemoryHeader::AttachHeader(sMemoryPool.Pop()));
        new (memory) T(std::forward<Args>(args)...);
        return memory;
    }

    static void Push(T* object)
    {
        object->~T();
        sMemoryPool.Push(MemoryHeader::DetachHeader(object));
    }

private:
    static constexpr UInt64     kAllocSize = sizeof(MemoryHeader) + sizeof(T);

private:
    static MemoryPool           sMemoryPool;
};

template<typename T>
MemoryPool       ObjectPool<T>::sMemoryPool(kAllocSize);

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
