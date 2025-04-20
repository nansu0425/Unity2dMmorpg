/*    ServerEngine/Core/Tls.h    */

#pragma once

class BlockMemoryPoolManager;
class TlsBlockMemoryPoolManager;

extern thread_local Int32                       tThreadId;
extern thread_local Stack<Int32>                tLockStack;
// extern thread_local BlockMemoryPoolManager*     tBlockMemoryPoolManager;
extern thread_local TlsBlockMemoryPoolManager*  tBlockMemoryPoolManager;
