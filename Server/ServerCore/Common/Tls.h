/*    ServerCore/Common/Tls.h    */

#pragma once

class JobQueue;
class SendChunk;

extern thread_local Int32                       tThreadId;
extern thread_local Stack<Int32>                tLockStack;
extern thread_local SharedPtr<SendChunk>        tSendChunk;
extern thread_local WeakPtr<JobQueue>           tFlushingQueue;
extern thread_local UInt64                      tWorkerLoopTick;
