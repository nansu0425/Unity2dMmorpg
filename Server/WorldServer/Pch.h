/*    WorldServer/Pch.h    */

#pragma once

#ifdef _DEBUG
#pragma comment(lib, "Debug//ServerCore.lib")
#pragma comment(lib, "Debug//GameContent.lib")
#else
#pragma comment(lib, "Release//ServerCore.lib")
#pragma comment(lib, "Release//GameContent.lib")
#endif // _DEBUG

#include "ServerCore/Common/Pch.h"
