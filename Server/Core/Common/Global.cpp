/*    Core/Common/Global.cpp    */

#include "Core/Pch.h"
#include "Core/Log/Logger.h"
#include "Core/Concurrency/Thread.h"
#include "Core/Concurrency/Deadlock.h"
#include "Core/Network/Socket.h"

namespace core
{
    Logger* gLogger = nullptr;
    ThreadManager* gThreadManager = nullptr;
    DeadlockDetector* gDeadlockDetector = nullptr;
    SendChunkPool* gSendChunkPool = nullptr;
    JobQueueManager* gJobQueueManager = nullptr;
    JobTimer* gJobTimer = nullptr;

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
} // namespace core
