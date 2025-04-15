/*    ServerEngine/Memory/Object.h    */

#pragma once

#include "ServerEngine/Memory/Allocator.h"

/*
 * 오브젝트 메모리는 클래스 별 전용 MemoryPool로 관리
 * USE_STOMP_ALLOCATOR가 true면, StompAllocator로 메모리 관리
 * 스마트 포인터의 custom allocator로 사용
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

    
public:
    using value_type = T;

    constexpr ObjectPool() noexcept                         {}
    template<typename U>
    constexpr ObjectPool(const ObjectPool<U>&) noexcept     {}

    // 스마트 포인터의 메모리 할당 인터페이스 구현
    __declspec(allocator) T* allocate(const UInt64 count)
    {
        ASSERT_CRASH_DEBUG(count == 1, "TOO_MANY_COUNT");

#if USE_STOMP_ALLOCATOR
        MemoryHeader* header = static_cast<MemoryHeader*>(StompAllocator::Alloc(kAllocSize));
        T* memory = static_cast<T*>(MemoryHeader::AttachHeader(header));
#else
        T* memory = static_cast<T*>(MemoryHeader::AttachHeader(sMemoryPool.Pop()));
#endif // USE_STOMP_ALLOCATOR
        
        return memory;
    }

    // 스마트 포인터의 메모리 해제 인터페이스 구현
    void deallocate(T* const object, const UInt64 count) noexcept
    {
        ASSERT_CRASH_DEBUG(count == 1, "TOO_MANY_COUNT");

#if USE_STOMP_ALLOCATOR
        StompAllocator::Free(MemoryHeader::DetachHeader(object));
#else
        sMemoryPool.Push(MemoryHeader::DetachHeader(object));
#endif // USE_STOMP_ALLOCATOR
    }

public:
    template<typename... Args>
    static SharedPtr<T> MakeShared(Args&&... args)
    {
        return std::allocate_shared<T>(ObjectPool<T>(), std::forward<Args>(args)...);
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
