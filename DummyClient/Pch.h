/*    DummyClient/Pch.h    */

#pragma once

#ifdef _DEBUG
#pragma comment(lib, "Debug//ServerEngine.lib")
#pragma comment(lib, "Debug//GameContent.lib")
#else
#pragma comment(lib, "Release//ServerEngine.lib")
#pragma comment(lib, "Release//GameContent.lib")
#endif // _DEBUG

#include "ServerEngine/Common/Pch.h"
