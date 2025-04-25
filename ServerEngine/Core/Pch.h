/*    ServerEngine/Core/Pch.h    */

#pragma once

// Windows
#define WIN32_LEAN_AND_MEAN
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

// ServerEngine
#include "ServerEngine/Core/Macro.h"
#include "ServerEngine/Core/Types.h"
#include "ServerEngine/Core/Global.h"
#include "ServerEngine/Core/Tls.h"
#include "ServerEngine/Log/Logger.h"
#include "ServerEngine/Concurrency/Lock.h"
#include "ServerEngine/Network/Buffer.h"
