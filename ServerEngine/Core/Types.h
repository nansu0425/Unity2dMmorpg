/*    ServerEngine/Core/Types.h    */

#pragma once

using Byte = unsigned char;
using Char = wchar_t;
using Int8 = __int8;
using Int16 = __int16;
using Int32 = __int32;
using Int64 = __int64;
using Float32 = float;
using Float64 = double;

template<typename T>
using Atomic = std::atomic<T>;
using Mutex = std::shared_mutex;
using CondVar = std::condition_variable;
using Thread = std::thread;
using ThreadId = std::thread::id;
using LockGuard = std::lock_guard<Mutex>;
using UniqueLock = std::unique_lock<Mutex>;
using SharedLock = std::shared_lock<Mutex>;
