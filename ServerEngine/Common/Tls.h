/*    ServerEngine/Common/Tls.h    */

#pragma once

class JobQueue;

extern thread_local Int32                       tThreadId;
extern thread_local Stack<Int32>                tLockStack;
extern thread_local WeakPtr<JobQueue>           tReservedJobs;
extern thread_local Int64                       tWorkerLoopTick;
