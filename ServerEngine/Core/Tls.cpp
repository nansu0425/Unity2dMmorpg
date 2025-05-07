/*    ServerEngine/Core/Tls.cpp    */

#include "ServerEngine/Pch.h"

thread_local Int32                      tThreadId = 0;
thread_local Stack<Int32>               tLockStack;
thread_local WeakPtr<JobQueue>          tReservedJobs;
thread_local Int64                      tWorkerLoopTick = 0;
