/*    ServerEngine/Core/Tls.cpp    */

#include "ServerEngine/Pch.h"

thread_local Int16 tThreadId = 0;
thread_local Stack<Int32> tLockStack;
