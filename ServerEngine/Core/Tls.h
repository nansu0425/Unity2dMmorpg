/*    ServerEngine/Core/Tls.h    */

#pragma once

extern thread_local Int32               tThreadId;
extern thread_local std::stack<Int32>   tLockStack;
