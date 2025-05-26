/*    ServerProtocol/Pch.h    */

#pragma once

#ifdef _DEBUG
#pragma comment(lib, "Debug//ServerCore.lib")
#else
#pragma comment(lib, "Release//ServerCore.lib")
#endif // _DEBUG

#include "ServerCore/Common/Pch.h"
