/*    ServerEngine/Core/Global.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Log/Logger.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Concurrency/Deadlock.h"
#include "ServerEngine/Network/Socket.h"

Logger*                 gLogger = nullptr;
ThreadManager*          gThreadManager = nullptr;
DeadlockDetector*       gDeadlockDetector = nullptr;
// SendBufferManager*      gSendBufferManager = nullptr;

GlobalContext::GlobalContext()
{
    gLogger = new Logger(TEXT_8("GlobalLogger"));
    gThreadManager = new ThreadManager();
    gDeadlockDetector = new DeadlockDetector();
    SocketUtils::Init();
    // gSendBufferManager = new SendBufferManager();
}

GlobalContext::~GlobalContext()
{
    // delete gSendBufferManager;
    SocketUtils::Cleanup();
    delete gDeadlockDetector;
    delete gThreadManager;
    delete gLogger;
}

GlobalContext   gGlobalContext;
