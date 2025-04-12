/*    ServerEngine/Core/Pch.h    */

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <iostream>
#include <shared_mutex>
#include <thread>
#include <atomic>
#include <vector>
#include <functional>
#include <memory>

#include "ServerEngine/Core/Types.h"
#include "ServerEngine/Core/Global.h"
#include "ServerEngine/Core/Tls.h"
#include "ServerEngine/Core/Macro.h"
