/*    ServerCore/Common/Pch.h    */

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

// ServerCore
#include "ServerCore/Common/Macro.h"
#include "ServerCore/Common/Types.h"
#include "ServerCore/Common/Global.h"
#include "ServerCore/Common/Tls.h"
#include "ServerCore/Log/Logger.h"
#include "ServerCore/Concurrency/Lock.h"
#include "ServerCore/Network/Buffer.h"
#include "ServerCore/Job/Serializer.h"
