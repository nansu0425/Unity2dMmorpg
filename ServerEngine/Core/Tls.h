/*    ServerEngine/Core/Tls.h    */

#pragma once

class BlockMemoryPoolManager;

extern thread_local Int32                       tThreadId;
extern thread_local Stack<Int32>                tLockStack;
extern thread_local BlockMemoryPoolManager      tBlockMemoryPoolManager;
