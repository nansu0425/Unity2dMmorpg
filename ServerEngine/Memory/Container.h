/*    ServerEngine/Memory/Container.h    */

#pragma once

template<typename T>
using Vector    = std::vector<T, ContainerAllocator<T>>;
template<typename T>
using List      = std::list<T, ContainerAllocator<T>>;

template<typename K, typename V, typename Pr = std::less<K>>
using TreeMap   = std::map<K, V, Pr, ContainerAllocator<std::pair<const K, V>>>;
template<typename K, typename V, typename Hasher = std::hash<K>, typename KeyEq = std::equal_to<K>>
using HashMap   = std::unordered_map<K, V, Hasher, KeyEq, ContainerAllocator<std::pair<const K, V>>>;
template<typename K, typename Pr = std::less<K>>
using TreeSet   = std::set<K, Pr, ContainerAllocator<K>>;
template<typename K, typename Hasher = std::hash<K>, typename KeyEq = std::equal_to<K>>
using HashSet   = std::unordered_set<K, Hasher, KeyEq, ContainerAllocator<K>>;

template<typename T>
using Deque     = std::deque<T, ContainerAllocator<T>>;
template<typename T, typename Container = Deque<T>>
using Queue     = std::queue<T, Container>;
template<typename T, typename Container = Deque<T>>
using Stack     = std::stack<T, Container>;
template<typename T, typename Container = Vector<T>, typename Pr = std::less<typename Container::value_type>>
using RankQueue = std::priority_queue<T, Container, Pr>;
