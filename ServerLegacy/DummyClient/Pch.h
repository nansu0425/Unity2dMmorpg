/*    DummyClient/Pch.h    */

#pragma once

#ifdef _DEBUG
#pragma comment(lib, "Debug//Core.lib")
#pragma comment(lib, "Debug//Protocol.lib")
#pragma comment(lib, "Debug//GameLogic.lib")
#else
#pragma comment(lib, "Release//Core.lib")
#pragma comment(lib, "Release//Protocol.lib")
#pragma comment(lib, "Release//GameLogic.lib")
#endif // _DEBUG

#include "Core/Common/Pch.h"
