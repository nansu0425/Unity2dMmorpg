/*    ServerEngine/Memory/ObjectPool.h    */

#pragma once

#include "ServerEngine/Memory/MemoryAllocator.h"

/*
 * 오브젝트 메모리를 클래스 별 전용 MemoryPool로 관리
 * USE_STOMP_ALLOCATOR가 true면, StompAllocator로 메모리 관리
 */
template<typename T>
class ObjectPool
{
public:
    static void Push(T* object)
    {
        object->~T();
#if USE_STOMP_ALLOCATOR
        StompMemoryAllocator::Free(MemoryHeader::DetachHeader(object));
#else
        sMemoryPool.Push(MemoryHeader::DetachHeader(object));
#endif // USE_STOMP_ALLOCATOR
    }

    template<typename... Args>
    static T* Pop(Args&&... args)
    {
#if USE_STOMP_ALLOCATOR
        MemoryHeader* header = static_cast<MemoryHeader*>(StompMemoryAllocator::Alloc(kAllocSize));
        T* memory = static_cast<T*>(MemoryHeader::AttachHeader(header));
#else
        T* memory = static_cast<T*>(MemoryHeader::AttachHeader(sMemoryPool.Pop()));
#endif // USE_STOMP_ALLOCATOR
        new (memory) T(std::forward<Args>(args)...);
        return memory;
    }

    static MemoryPool&          GetMemoryPool() { return sMemoryPool; }

private:
    static constexpr UInt64     kAllocSize = sizeof(MemoryHeader) + sizeof(T);

private:
    static MemoryPool           sMemoryPool;
};

template<typename T>
MemoryPool       ObjectPool<T>::sMemoryPool(kAllocSize);
