/*    ServerEngine/Memory/Object.h    */

#pragma once

#include "ServerEngine/Memory/Allocator.h"

/*
 * 클래스 별로 별도의 메모리 풀을 갖게 된다
 * 오브젝트의 메모리는 메모리 풀에 의해 관리된다
 */
template<typename T>
class ObjectPool
{
public:
    static void Push(T* object)
    {
        object->~T();
#if USE_STOMP_ALLOCATOR
        StompAllocator::Free(MemoryHeader::DetachHeader(object));
#else
        sMemoryPool.Push(MemoryHeader::DetachHeader(object));
#endif // USE_STOMP_ALLOCATOR
    }

    template<typename... Args>
    static T* Pop(Args&&... args)
    {
#if USE_STOMP_ALLOCATOR
        MemoryHeader* header = static_cast<MemoryHeader*>(StompAllocator::Alloc(kAllocSize));
        T* memory = static_cast<T*>(MemoryHeader::AttachHeader(header));
#else
        T* memory = static_cast<T*>(MemoryHeader::AttachHeader(sMemoryPool.Pop()));
#endif // USE_STOMP_ALLOCATOR
        new (memory) T(std::forward<Args>(args)...);
        return memory;
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
    T* memory = static_cast<T*>(PoolAllocator::Alloc(sizeof(T)));
    new (memory) T(std::forward<Args>(args)...);
    return memory;
}

template<typename T>
inline void DeleteObject(T* object)
{
    object->~T();
    PoolAllocator::Free(object);
}
