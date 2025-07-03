/*    GameServer/Pch.h    */

#pragma once

#ifdef _DEBUG
#pragma comment(lib, "Debug//Core.lib")
#pragma comment(lib, "Debug//Protocol.lib")
#else
#pragma comment(lib, "Release//Core.lib")
#pragma comment(lib, "Release//Protocol.lib")
#endif // _DEBUG

#include "Core/Common/Pch.h"
