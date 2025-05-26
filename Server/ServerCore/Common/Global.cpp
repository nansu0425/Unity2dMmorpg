/*    ServerCore/Common/Global.cpp    */

#include "ServerCore/Pch.h"
#include "ServerCore/Log/Logger.h"
#include "ServerCore/Concurrency/Thread.h"
#include "ServerCore/Concurrency/Deadlock.h"
#include "ServerCore/Network/Socket.h"

Logger*                 gLogger = nullptr;
ThreadManager*          gThreadManager = nullptr;
DeadlockDetector*       gDeadlockDetector = nullptr;
SendChunkPool*          gSendChunkPool = nullptr;
JobQueueManager*        gJobQueueManager = nullptr;
JobTimer*               gJobTimer = nullptr;

GlobalContext::GlobalContext()
{
    gLogger = new Logger(TEXT_8("GlobalLogger"));
    gThreadManager = new ThreadManager();
    gDeadlockDetector = new DeadlockDetector();
    gSendChunkPool = new SendChunkPool();
    SocketUtils::Init();
    gJobQueueManager = new JobQueueManager();
    gJobTimer = new JobTimer();
}

GlobalContext::~GlobalContext()
{
    delete gJobTimer;
    delete gJobQueueManager;
    SocketUtils::Cleanup();
    delete gSendChunkPool;
    delete gDeadlockDetector;
    delete gThreadManager;
    delete gLogger;
}

GlobalContext   gGlobalContext;
