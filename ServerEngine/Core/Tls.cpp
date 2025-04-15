/*    ServerEngine/Core/Tls.cpp    */

#include "ServerEngine/Pch.h"

thread_local Int32              tThreadId = 0;
thread_local std::stack<Int32>  tLockStack;
