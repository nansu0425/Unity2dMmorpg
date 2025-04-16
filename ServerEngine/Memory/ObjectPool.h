/*    ServerEngine/Memory/ObjectPool.h    */

#pragma once

#include "ServerEngine/Memory/MemoryAllocator.h"

class ObjectPoolUniquePtrDeleter;

/*
 * 오브젝트 메모리를 클래스 별 전용 MemoryPool로 관리
 * USE_STOMP_ALLOCATOR가 true면, StompMemoryAllocator로 메모리 관리
 */
template<typename T>
class ObjectPool
{
public:
    static void Push(T* object)
    {
        object->~T();
#if USE_STOMP_ALLOCATOR
        MemoryHeader* header = MemoryHeader::DetachHeader(object);
        ASSERT_CRASH_DEBUG(header->allocSize == kAllocSizeT, "INVALID_ALLOC_SIZE");
        StompMemoryAllocator::Free(header);
#else
        sMemoryPool.Push(MemoryHeader::DetachHeader(object));
#endif // USE_STOMP_ALLOCATOR
    }

    template<typename... Args>
    static T* Pop(Args&&... args)
    {
#if USE_STOMP_ALLOCATOR
        MemoryHeader* header = static_cast<MemoryHeader*>(StompMemoryAllocator::Alloc(kAllocSizeT));
        header->allocSize = kAllocSizeT;
        T* memory = static_cast<T*>(MemoryHeader::AttachHeader(header));
#else
        T* memory = static_cast<T*>(MemoryHeader::AttachHeader(sMemoryPool.Pop()));
#endif // USE_STOMP_ALLOCATOR
        new (memory) T(std::forward<Args>(args)...);

        return memory;
    }

    static MemoryPool&  GetMemoryPool() { return sMemoryPool; }

public:
    /*
     * 오브젝트 풀을 사용하는 스마트 포인터 메모리 할당기
     */
    template<typename U = T>
    class SmartPointerAllocator
    {
    public:
        using value_type = U;

        constexpr SmartPointerAllocator() noexcept                                {}
        template<typename V>
        constexpr SmartPointerAllocator(const SmartPointerAllocator<V>&) noexcept {}

        __declspec(allocator) U* allocate(const UInt64 count)
        {
            ASSERT_CRASH_DEBUG(count == 1, "TOO_MANY_COUNT");

#if USE_STOMP_ALLOCATOR
            MemoryHeader* header = static_cast<MemoryHeader*>(StompMemoryAllocator::Alloc(kAllocSizeU));
            header->allocSize = kAllocSizeU;
            U* memory = static_cast<U*>(MemoryHeader::AttachHeader(header));
#else
            U* memory = static_cast<U*>(MemoryHeader::AttachHeader(ObjectPool<U>::GetMemoryPool().Pop()));
#endif // USE_STOMP_ALLOCATOR

            return memory;
        }

        void deallocate(U* const object, const UInt64 count) noexcept
        {
            ASSERT_CRASH_DEBUG(count == 1, "TOO_MANY_COUNT");

#if USE_STOMP_ALLOCATOR
            MemoryHeader* header = MemoryHeader::DetachHeader(object);
            ASSERT_CRASH_DEBUG(header->allocSize == kAllocSizeU, "INVALID_ALLOC_SIZE");
            StompMemoryAllocator::Free(header);
#else
            ObjectPool<U>::GetMemoryPool().Push(MemoryHeader::DetachHeader(object));
#endif // USE_STOMP_ALLOCATOR
        }

    private:
#if USE_STOMP_ALLOCATOR
        static constexpr UInt64     kAllocSizeU = sizeof(MemoryHeader) + sizeof(U);
#endif
    };

public:
    using SharedPtr             = std::shared_ptr<T>;
    using UniquePtr             = std::unique_ptr<T, ObjectPoolUniquePtrDeleter>;

    template<typename... Args>
    static SharedPtr            MakeShared(Args&&... args);
    template<typename... Args>
    static UniquePtr            MakeUnique(Args&&... args);

private:
    static constexpr UInt64     kAllocSizeT = sizeof(MemoryHeader) + sizeof(T);

private:
    static MemoryPool           sMemoryPool;
};

template<typename T>
MemoryPool       ObjectPool<T>::sMemoryPool(kAllocSizeT);

/*
 * 오브젝트 풀 UniquePtr의 deleter로 사용
 */
class ObjectPoolUniquePtrDeleter
{
private:
    using DeleterFunction = void (*)(void*);

public:
    ObjectPoolUniquePtrDeleter() : mDeleterFunction(DeleteNothing)                                              {}
    explicit ObjectPoolUniquePtrDeleter(DeleterFunction deleterFunction) : mDeleterFunction(deleterFunction)    {}

    // UniquePtr가 객체를 소멸할 때 호출
    void operator()(void* object) const         { mDeleterFunction(object); }
    // 타입에 맞는 UniquePtrDeleter 생성
    template<typename T>
    static ObjectPoolUniquePtrDeleter Create()  { return ObjectPoolUniquePtrDeleter(DeleteObject<T>); }

private:
    // UniquePtr가 nullptr일 때 설정되는 Deleter 함수
    static void         DeleteNothing(void* object) noexcept
    {}
    // 객체 소멸 작업을 수행
    template<typename T>
    static void         DeleteObject(void* object) noexcept
    {
        static_cast<T*>(object)->~T();
        ObjectPool<T>::SmartPointerAllocator().deallocate(static_cast<T*>(object), 1);
    }

private:
    DeleterFunction     mDeleterFunction = nullptr;

};

template<typename T>
template<typename... Args>
inline typename ObjectPool<T>::SharedPtr ObjectPool<T>::MakeShared(Args&&... args)
{
    return std::allocate_shared<T>(SmartPointerAllocator(), std::forward<Args>(args)...);
}

template<typename T>
template<typename... Args>
inline typename ObjectPool<T>::UniquePtr ObjectPool<T>::MakeUnique(Args&&... args)
{
    T* memory = SmartPointerAllocator().allocate(1);
    new (memory) T(std::forward<Args>(args)...);
    return UniquePtr(memory, ObjectPoolUniquePtrDeleter::Create<T>());
}
