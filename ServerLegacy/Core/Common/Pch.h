/*    Core/Common/Pch.h    */

#pragma once

// Windows
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

// STL
#include <iostream>
#include <shared_mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>
#include <queue>
#include <stack>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <chrono>
#include <string>
#include <string_view>

// moodycamel
#include <concurrentqueue/concurrentqueue.h>

// Core
#include "Core/Common/Macro.h"
#include "Core/Common/Types.h"
#include "Core/Common/Global.h"
#include "Core/Common/Tls.h"
#include "Core/Log/Logger.h"
#include "Core/Concurrency/Lock.h"
#include "Core/Network/Buffer.h"
#include "Core/Job/Serializer.h"
