/*    GameContent/Pch.h    */

#pragma once

#ifdef _DEBUG
#pragma comment(lib, "Debug//ServerEngine.lib")
#else
#pragma comment(lib, "Release//ServerEngine.lib")
#endif // _DEBUG

#include "ServerEngine/Common/Pch.h"
