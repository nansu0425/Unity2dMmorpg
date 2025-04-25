/*    ServerEngine/Core/Global.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Log/Logger.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Concurrency/Deadlock.h"
#include "ServerEngine/Network/Socket.h"

ThreadManager*           gThreadManager = nullptr;
DeadlockDetector*        gDeadlockDetector = nullptr;
SharedPtr<Logger>        gLogger = nullptr;

GlobalContext::GlobalContext()
{
    gLogger = LoggerUtils::CreateLogger();
    gThreadManager = new ThreadManager();
    gDeadlockDetector = new DeadlockDetector();
    SocketUtils::Init();
}

GlobalContext::~GlobalContext()
{
    SocketUtils::Cleanup();
    delete gDeadlockDetector;
    delete gThreadManager;
    LoggerUtils::ShutdownLogger();
}

GlobalContext*          gGlobalContext;
