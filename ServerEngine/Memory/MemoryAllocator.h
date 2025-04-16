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
 * Stomp Memory : [|    Page|    ][    Page    ]   ...   [    Page    ]|
 * Data         : [|       [|                  Data                   ]|
 * Address      :  |base    |base + offset                             |base + size
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

public:
    template<typename T>
    class ContainerInterface
    {
    public:
        using value_type = T;

        constexpr ContainerInterface() noexcept                             {}
        template<typename U>
        constexpr ContainerInterface(const ContainerInterface<U>&) noexcept {}

        __declspec(allocator) T* allocate(const UInt64 count)               { return static_cast<T*>(PoolAllocator::Alloc(count * sizeof(T))); }
        void deallocate(T* const base, const UInt64 count) noexcept         { PoolAllocator::Free(base); }
    };

    template<typename T>
    using Vector    = std::vector<T, ContainerInterface<T>>;
    template<typename T>
    using List      = std::list<T, ContainerInterface<T>>;

    template<typename K, typename V, typename Pr = std::less<K>>
    using TreeMap   = std::map<K, V, Pr, ContainerInterface<std::pair<const K, V>>>;
    template<typename K, typename V, typename Hasher = std::hash<K>, typename KeyEq = std::equal_to<K>>
    using HashMap   = std::unordered_map<K, V, Hasher, KeyEq, ContainerInterface<std::pair<const K, V>>>;
    template<typename K, typename Pr = std::less<K>>
    using TreeSet   = std::set<K, Pr, ContainerInterface<K>>;
    template<typename K, typename Hasher = std::hash<K>, typename KeyEq = std::equal_to<K>>
    using HashSet   = std::unordered_set<K, Hasher, KeyEq, ContainerInterface<K>>;

    template<typename T>
    using Deque     = std::deque<T, ContainerInterface<T>>;
    template<typename T, typename Container = Deque<T>>
    using Queue     = std::queue<T, Container>;
    template<typename T, typename Container = Deque<T>>
    using Stack     = std::stack<T, Container>;
    template<typename T, typename Container = Vector<T>, typename Pr = std::less<typename Container::value_type>>
    using RankQueue = std::priority_queue<T, Container, Pr>;

    using String8   = std::basic_string<Char8, std::char_traits<Char8>, ContainerInterface<Char8>>;
    using String16  = std::basic_string<Char16, std::char_traits<Char16>, ContainerInterface<Char16>>;
};

template<typename L, typename R>
bool operator==(const PoolAllocator::ContainerInterface<L>&, const PoolAllocator::ContainerInterface<R>&) noexcept { return true; }
template<typename L, typename R>
bool operator!=(const PoolAllocator::ContainerInterface<L>&, const PoolAllocator::ContainerInterface<R>&) noexcept { return false; }
