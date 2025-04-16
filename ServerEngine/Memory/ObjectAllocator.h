#pragma once

#include "ServerEngine/Memory/ObjectPool.h"

class ObjectPoolAllocatorDeleter;

template<typename T>
class ObjectPoolAllocator
{
public:
    using value_type = T;

    constexpr ObjectPoolAllocator() noexcept                                {}
    template<typename U>
    constexpr ObjectPoolAllocator(const ObjectPoolAllocator<U>&) noexcept   {}

    // 스마트 포인터의 메모리 할당 인터페이스
    __declspec(allocator) T* allocate(const UInt64 count)
    {
        ASSERT_CRASH_DEBUG(count == 1, "TOO_MANY_COUNT");

#if USE_STOMP_ALLOCATOR
        MemoryHeader* header = static_cast<MemoryHeader*>(StompMemoryAllocator::Alloc(kAllocSize));
        T* memory = static_cast<T*>(MemoryHeader::AttachHeader(header));
#else
        T* memory = static_cast<T*>(MemoryHeader::AttachHeader(ObjectPool<T>::GetMemoryPool().Pop()));
#endif // USE_STOMP_ALLOCATOR

        return memory;
    }

    // 스마트 포인터의 메모리 해제 인터페이스
    void deallocate(T* const object, const UInt64 count) noexcept
    {
        ASSERT_CRASH_DEBUG(count == 1, "TOO_MANY_COUNT");

#if USE_STOMP_ALLOCATOR
        StompMemoryAllocator::Free(MemoryHeader::DetachHeader(object));
#else
        ObjectPool<T>::GetMemoryPool().Push(MemoryHeader::DetachHeader(object));
#endif // USE_STOMP_ALLOCATOR
    }

public:
    using SharedPtr     = std::shared_ptr<T>;
    using UniquePtr     = std::unique_ptr<T, ObjectPoolAllocatorDeleter>;

    template<typename... Args>
    static SharedPtr    MakeShared(Args&&... args);
    template<typename... Args>
    static UniquePtr    MakeUnique(Args&&... args);
};

/*
 * ObjectPoolAllocator<T>::UniquePtr의 deleter로 사용하기 위한 어댑터
 */
class ObjectPoolAllocatorDeleter
{
private:
    using DeleterFunction = void (*)(void*);

public:
    ObjectPoolAllocatorDeleter() : mDeleteFunction(DeleteNothing)                                           {}
    explicit ObjectPoolAllocatorDeleter(DeleterFunction deleterFunction) : mDeleteFunction(deleterFunction) {}

    // UniquePtr가 객체를 소멸할 때 호출
    void operator()(void* object) const         { mDeleteFunction(object); }
    // 타입에 맞는 UniquePtrDeleter 생성
    template<typename T>
    static ObjectPoolAllocatorDeleter Create()  { return ObjectPoolAllocatorDeleter(DeleteObject<T>); }

private:
    // UniquePtr가 nullptr일 때 설정되는 Deleter 함수
    static void         DeleteNothing(void* object) noexcept
    {}
    // 객체 소멸 작업을 수행
    template<typename T>
    static void         DeleteObject(void* object) noexcept
    {
        // 소멸자 호출 후 메모리 해제
        static_cast<T*>(object)->~T();
        ObjectPoolAllocator<T>().deallocate(static_cast<T*>(object), 1);
    }

private:
    DeleterFunction     mDeleteFunction = nullptr;

};

template<typename T>
template<typename... Args>
inline typename ObjectPoolAllocator<T>::SharedPtr ObjectPoolAllocator<T>::MakeShared(Args&&... args)
{
    return std::allocate_shared<T>(ObjectPoolAllocator(), std::forward<Args>(args)...);
}

template<typename T>
template<typename... Args>
inline typename ObjectPoolAllocator<T>::UniquePtr ObjectPoolAllocator<T>::MakeUnique(Args&&... args)
{
    T* memory = ObjectPoolAllocator().allocate(1);
    new (memory) T(std::forward<Args>(args)...);
    return UniquePtr(memory, ObjectPoolAllocatorDeleter::Create<T>());
}
