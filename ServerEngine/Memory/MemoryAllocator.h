/*    ServerEngine/Memory/MemoryAllocator.h    */

#pragma once

#include "ServerEngine/Memory/MemoryPool.h"

class BaseMemoryAllocator
{
public:
    static void*    Alloc(UInt64 size);
    static void     Free(void* memory);
};

/*
 * Stomp Memory : [|    Page|    ][    Page    ]   ...   [    Page    ]|
 * Data         : [|       [|                  Data                   ]|
 * Address      :  |base    |base + offset                             |base + size
 * USE_STOMP_ALLOCATOR 매크로를 true로 설정하면 메모리 풀 사용 시 Stomp 메모리 할당기 사용
 */
class StompMemoryAllocator
{
public:
    static void*    Alloc(UInt64 size);
    static void     Free(void* memory);

private:
    static constexpr UInt64     kPageSize = 4096;
};

/*
 * 전역 메모리 풀 매니저를 사용하는 메모리 할당기
 */
class PoolMemoryAllocator
{
public:
    static void*    Alloc(UInt64 size);
    static void     Free(void* memory);

public:
    template<typename T>
    class ContainerAdapter
    {
    public:
        using value_type = T;

        constexpr ContainerAdapter() noexcept                           {}
        template<typename U>
        constexpr ContainerAdapter(const ContainerAdapter<U>&) noexcept {}

        __declspec(allocator) T* allocate(const UInt64 count)           { return static_cast<T*>(Alloc(count * sizeof(T))); }
        void deallocate(T* const memory, const UInt64 count) noexcept   { Free(memory); }
    };

    template<typename T>
    using Vector    = std::vector<T, ContainerAdapter<T>>;
    template<typename T>
    using List      = std::list<T, ContainerAdapter<T>>;

    template<typename K, typename V, typename Pr = std::less<K>>
    using TreeMap   = std::map<K, V, Pr, ContainerAdapter<std::pair<const K, V>>>;
    template<typename K, typename V, typename Hasher = std::hash<K>, typename KeyEq = std::equal_to<K>>
    using HashMap   = std::unordered_map<K, V, Hasher, KeyEq, ContainerAdapter<std::pair<const K, V>>>;
    template<typename K, typename Pr = std::less<K>>
    using TreeSet   = std::set<K, Pr, ContainerAdapter<K>>;
    template<typename K, typename Hasher = std::hash<K>, typename KeyEq = std::equal_to<K>>
    using HashSet   = std::unordered_set<K, Hasher, KeyEq, ContainerAdapter<K>>;

    template<typename T>
    using Deque     = std::deque<T, ContainerAdapter<T>>;
    template<typename T, typename Container = Deque<T>>
    using Queue     = std::queue<T, Container>;
    template<typename T, typename Container = Deque<T>>
    using Stack     = std::stack<T, Container>;
    template<typename T, typename Container = Vector<T>, typename Pr = std::less<typename Container::value_type>>
    using RankQueue = std::priority_queue<T, Container, Pr>;

    using String8   = std::basic_string<Char8, std::char_traits<Char8>, ContainerAdapter<Char8>>;
    using String16  = std::basic_string<Char16, std::char_traits<Char16>, ContainerAdapter<Char16>>;
};

/*
 * 오브젝트 타입별 전용 메모리 풀을 사용하는 메모리 할당기
 */
template<typename T>
class ObjectPoolMemoryAllocator
{
public:
    using value_type = T;

    constexpr ObjectPoolMemoryAllocator() noexcept                                      {}
    template<typename U>
    constexpr ObjectPoolMemoryAllocator(const ObjectPoolMemoryAllocator<U>&) noexcept   {}

    __declspec(allocator) T* allocate(const UInt64 count)
    {
        ASSERT_CRASH_DEBUG(count == 1, "TOO_MANY_COUNT");

#if USE_STOMP_ALLOCATOR
        MemoryHeader* header = static_cast<MemoryHeader*>(StompMemoryAllocator::Alloc(kAllocSize));
        header->allocSize = kAllocSize;
        T* memory = static_cast<T*>(MemoryHeader::AttachHeader(header));
#else
        T* memory = static_cast<T*>(MemoryHeader::AttachHeader(sMemoryPool.Pop()));
#endif // USE_STOMP_ALLOCATOR

        return memory;
    }

    void deallocate(T* const memory, const UInt64 count) noexcept
    {
        ASSERT_CRASH_DEBUG(count == 1, "TOO_MANY_COUNT");

#if USE_STOMP_ALLOCATOR
        MemoryHeader* header = MemoryHeader::DetachHeader(memory);
        ASSERT_CRASH_DEBUG(header->allocSize == kAllocSize, "INVALID_ALLOC_SIZE");
        StompMemoryAllocator::Free(header);
#else
        sMemoryPool.Push(MemoryHeader::DetachHeader(memory));
#endif // USE_STOMP_ALLOCATOR
    }


private:
    static constexpr UInt64     kAllocSize = sizeof(MemoryHeader) + sizeof(T);

private:
    static MemoryPool           sMemoryPool;
};

template<typename T>
MemoryPool      ObjectPoolMemoryAllocator<T>::sMemoryPool(kAllocSize);
