/*    ServerCore/Common/Tls.cpp    */

#include "ServerCore/Pch.h"

thread_local Int32                      tThreadId = 0;
thread_local Stack<Int32>               tLockStack;
thread_local SharedPtr<SendChunk>       tSendChunk;
thread_local WeakPtr<JobQueue>          tFlushingQueue;
thread_local UInt64                     tWorkerLoopTick = 0;
