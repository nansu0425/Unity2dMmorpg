/*    ServerEngine/Common/Global.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Log/Logger.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Concurrency/Deadlock.h"
#include "ServerEngine/Network/Socket.h"
#include "ServerEngine/Job/Timer.h"

Logger*                 gLogger = nullptr;
ThreadManager*          gThreadManager = nullptr;
DeadlockDetector*       gDeadlockDetector = nullptr;
JobQueueManager*        gJobQueueManager = nullptr;
JobTimer*               gJobTimer = nullptr;

GlobalContext::GlobalContext()
{
    gLogger = new Logger(TEXT_8("GlobalLogger"));
    gThreadManager = new ThreadManager();
    gDeadlockDetector = new DeadlockDetector();
    SocketUtils::Init();
    gJobQueueManager = new JobQueueManager();
    gJobTimer = new JobTimer();
}

GlobalContext::~GlobalContext()
{
    delete gJobTimer;
    delete gJobQueueManager;
    SocketUtils::Cleanup();
    delete gDeadlockDetector;
    delete gThreadManager;
    delete gLogger;
}

GlobalContext   gGlobalContext;
