/*    ServerEngine/Core/Types.h    */

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
using UniquePtr     = std::unique_ptr<T>;

using String8View   = std::basic_string_view<Char8>;
using String16View  = std::basic_string_view<Char16>;
