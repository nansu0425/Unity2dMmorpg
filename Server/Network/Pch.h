/*    Network/Pch.h    */

#pragma once

#ifdef _DEBUG
#pragma comment(lib, "Debug//Core.lib")
#else
#pragma comment(lib, "Release//Core.lib")
#endif // _DEBUG

#include "Core/Pch.h"

namespace net
{
    void HelloWorld();
}
