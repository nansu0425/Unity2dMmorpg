/*    ServerEngine/Memory/ObjectAllocator.h    */

#pragma once

#include "ServerEngine/Memory/ObjectPool.h"

/*
 * 오브젝트 풀을 사용하는 오브젝트 할당기
 * 전용 스마트 포인터를 제공
 */
class PoolObjectAllocator
{
public:
    class UniquePtrDeleter
    {
    private:
        using DeleterFunction = void (*)(void*);

    public:
        UniquePtrDeleter() : mDeleterFunction(DeleteNothing)                                            {}
        explicit UniquePtrDeleter(DeleterFunction deleterFunction) : mDeleterFunction(deleterFunction)  {}

        // UniquePtr가 객체를 소멸할 때 호출
        void operator()(void* object) const     { mDeleterFunction(object); }
        // 타입에 맞는 UniquePtrDeleter 생성
        template<typename T>
        static UniquePtrDeleter Create()        { return UniquePtrDeleter(DeleteObject<T>); }

    private:
        // UniquePtr가 nullptr일 때 설정되는 DeleterFunction
        static void         DeleteNothing(void* object) noexcept
        {}
        // 오브젝트 풀에 넣는다
        template<typename T>
        static void         DeleteObject(void* object) noexcept
        {
            ObjectPool<T>::Push(static_cast<T*>(object));
        }

    private:
        DeleterFunction     mDeleterFunction = nullptr;
    };

public:
    template<typename T>
    using SharedPtr         = std::shared_ptr<T>;
    template<typename T>
    using UniquePtr         = std::unique_ptr<T, UniquePtrDeleter>;

    template<typename T, typename... Args>
    static SharedPtr<T>     MakeShared(Args&&... args)
    {
        return std::allocate_shared<T>(ObjectPoolMemoryAllocator<T>(), std::forward<Args>(args)...);
    }
    template<typename T, typename... Args>
    static UniquePtr<T>     MakeUnique(Args&&... args)
    {
        return UniquePtr<T>(ObjectPool<T>::Pop(std::forward<Args>(args)...), UniquePtrDeleter::Create<T>());
    }
};
