/*    ServerEngine/Memory/Object.h    */

#pragma once

#include "ServerEngine/Memory/MemoryAllocator.h"

class UniquePtrDeleter;

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
        MemoryHeader* header = static_cast<MemoryHeader*>(StompMemoryAllocator::Alloc(kAllocSize));
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
        StompMemoryAllocator::Free(MemoryHeader::DetachHeader(object));
#else
        sMemoryPool.Push(MemoryHeader::DetachHeader(object));
#endif // USE_STOMP_ALLOCATOR
    }
    
public: // ObjectPool을 사용하는 스마트 포인터 선언
    using SharedPtr     = std::shared_ptr<T>;
    using UniquePtr     = std::unique_ptr<T, UniquePtrDeleter>;

    template<typename... Args>
    static SharedPtr    MakeShared(Args&&... args);
    template<typename... Args>
    static UniquePtr    MakeUnique(Args&&... args);

private:
    static constexpr UInt64     kAllocSize = sizeof(MemoryHeader) + sizeof(T);

private:
    static MemoryPool           sMemoryPool;
};

template<typename T>
MemoryPool       ObjectPool<T>::sMemoryPool(kAllocSize);

/*
 * UniquePtr의 Deleter로 사용하기 위한 인터페이스 구현
 */
class UniquePtrDeleter
{
private:
    using DeleterFunction = void (*)(void*);

public:
    UniquePtrDeleter() : mDeleteObject(DeleteNothing)                                           {}
    explicit UniquePtrDeleter(DeleterFunction deleterFunction) : mDeleteObject(deleterFunction) {}

    // UniquePtr가 객체를 소멸할 때 호출
    void operator()(void* object) const { mDeleteObject(object); }
    // 타입에 맞는 UniquePtrDeleter 생성
    template<typename T>
    static UniquePtrDeleter Create()    { return UniquePtrDeleter(DeleteObject<T>); }

private:
    // UniquePtr가 nullptr일 때 설정되는 Deleter 함수
    static void     DeleteNothing(void* object) noexcept
    {}
    // 객체 소멸 작업을 수행
    template<typename T>
    static void     DeleteObject(void* object) noexcept
    {
        // 소멸자 호출 후 메모리 해제
        static_cast<T*>(object)->~T();
        ObjectPool<T>().deallocate(static_cast<T*>(object), 1);
    }

private:
    DeleterFunction     mDeleteObject = nullptr;

};

template<typename T, typename... Args>
inline T* NewObject(Args&&... args)
{
    T* memory = static_cast<T*>(MemoryPoolAllocator::Alloc(sizeof(T)));
    new (memory) T(std::forward<Args>(args)...);
    return memory;
}

template<typename T>
inline void DeleteObject(T* object)
{
    object->~T();
    MemoryPoolAllocator::Free(object);
}

template<typename T>
template<typename... Args>
inline typename ObjectPool<T>::SharedPtr ObjectPool<T>::MakeShared(Args&&... args)
{
    return std::allocate_shared<T>(ObjectPool(), std::forward<Args>(args)...);
}

template<typename T>
template<typename... Args>
inline typename ObjectPool<T>::UniquePtr ObjectPool<T>::MakeUnique(Args&&... args)
{
    T* memory = ObjectPool().allocate(1);
    new (memory) T(std::forward<Args>(args)...);
    return UniquePtr(memory, UniquePtrDeleter::Create<T>());
}
