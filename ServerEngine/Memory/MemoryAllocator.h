/*    ServerEngine/Memory/Allocator.h    */

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
 */
class StompMemoryAllocator
{
public:
    static void*    Alloc(UInt64 size);
    static void     Free(void* memory);

private:
    static constexpr UInt64     kPageSize = 4096;
};

class MemoryPoolAllocator
{
public:
    static void*    Alloc(UInt64 size);
    static void     Free(void* memory);

public:
    template<typename T, typename... Args>
    static T* New(Args&&... args)
    {
        T* memory = static_cast<T*>(Alloc(sizeof(T)));
        new (memory) T(std::forward<Args>(args)...);
        return memory;
    }

    template<typename T>
    static void Delete(T* object)
    {
        object->~T();
        Free(object);
    }

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
        void deallocate(T* const base, const UInt64 count) noexcept     { Free(base); }
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

template<typename L, typename R>
bool operator==(const MemoryPoolAllocator::ContainerAdapter<L>&, const MemoryPoolAllocator::ContainerAdapter<R>&) noexcept { return true; }
template<typename L, typename R>
bool operator!=(const MemoryPoolAllocator::ContainerAdapter<L>&, const MemoryPoolAllocator::ContainerAdapter<R>&) noexcept { return false; }
