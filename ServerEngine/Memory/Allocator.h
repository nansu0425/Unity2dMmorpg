/*    ServerEngine/Memory/Allocator.h    */

#pragma once

#include "ServerEngine/Memory/MemoryPool.h"

class BaseAllocator
{
public:
    static void*    Alloc(UInt64 size);
    static void     Free(void* memory);
};

/*
 * 할당된 메모리     : [|   페이지|   ][    페이지    ]   ...   [    페이지    ]|
 * 데이터 위치       : [|       [|                  데이터                   ]|
 * 주소             :  |base    |base + offset                              |base + size
 */
class StompAllocator
{
public:
    static void*    Alloc(UInt64 size);
    static void     Free(void* memory);

private:
    static constexpr UInt64     kPageSize = 4096;
};

class PoolAllocator
{
public:
    static void*    Alloc(UInt64 size);
    static void     Free(void* memory);
};

template<typename T>
class ContainerAllocator
{
public:
    using value_type = T;

    constexpr ContainerAllocator() noexcept                             {}
    template<typename U>
    constexpr ContainerAllocator(const ContainerAllocator<U>&) noexcept {}

    __declspec(allocator) T* allocate(const UInt64 count)               { return static_cast<T*>(PoolAllocator::Alloc(count * sizeof(T))); }
    void deallocate(T* const base, const UInt64 count) noexcept         { PoolAllocator::Free(base); }
};

template<typename L, typename R>
bool operator==(const ContainerAllocator<L>&, const ContainerAllocator<R>&) noexcept    { return true; }
template<typename L, typename R>
bool operator!=(const ContainerAllocator<L>&, const ContainerAllocator<R>&) noexcept    { return false; }
