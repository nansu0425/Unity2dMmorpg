/*    ServerCore/Common/Types.h    */

#pragma once

using Byte      = unsigned char;
using Bool      = bool;
using Char8     = char;
using Char16    = wchar_t;
using Int8      = __int8;
using Int16     = __int16;
using Int32     = __int32;
using Int64     = __int64;
using UInt8     = unsigned __int8;
using UInt16    = unsigned __int16;
using UInt32    = unsigned __int32;
using UInt64    = unsigned __int64;
using Float32   = float;
using Float64   = double;

template<typename T>
using Atomic        = std::atomic<T>;
using Mutex         = std::mutex;
using SharedMutex   = std::shared_mutex;
using LockGuard     = std::lock_guard<SharedMutex>;
using UniqueLock    = std::unique_lock<SharedMutex>;
using SharedLock    = std::shared_lock<SharedMutex>;
using CondVar       = std::condition_variable;
using Thread        = std::thread;

template<typename F>
using Function      = std::function<F>;

template<typename T>
using SharedPtr     = std::shared_ptr<T>;
template<typename T>
using WeakPtr       = std::weak_ptr<T>;
template<typename T>
using UniquePtr     = std::unique_ptr<T>;

template<typename T>
using Vector        = std::vector<T>;
template<typename T>
using List          = std::list<T>;

template<typename K, typename V, typename Pr = std::less<K>>
using TreeMap       = std::map<K, V, Pr>;
template<typename K, typename V, typename Hasher = std::hash<K>, typename KeyEq = std::equal_to<K>>
using HashMap       = std::unordered_map<K, V, Hasher, KeyEq>;
template<typename K, typename Pr = std::less<K>>
using TreeSet       = std::set<K, Pr>;
template<typename K, typename Hasher = std::hash<K>, typename KeyEq = std::equal_to<K>>
using HashSet       = std::unordered_set<K, Hasher, KeyEq>;

template<typename T>
using Deque         = std::deque<T>;
template<typename T, typename Container = Deque<T>>
using Queue         = std::queue<T, Container>;
template<typename T, typename Container = Deque<T>>
using Stack         = std::stack<T, Container>;
template<typename T, typename Container = Vector<T>, typename Pr = std::less<typename Container::value_type>>
using PriorityQueue = std::priority_queue<T, Container, Pr>;

template<typename T>
using LockfreeQueue = moodycamel::ConcurrentQueue<T>;

using String8       = std::basic_string<Char8, std::char_traits<Char8>>;
using String16      = std::basic_string<Char16, std::char_traits<Char16>>;
using String8View   = std::basic_string_view<Char8>;
using String16View  = std::basic_string_view<Char16>;
