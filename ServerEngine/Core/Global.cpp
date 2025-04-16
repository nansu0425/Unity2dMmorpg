/*    ServerEngine/Core/Global.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Concurrency/Deadlock.h"
#include "ServerEngine/Memory/MemoryPool.h"
#include "ServerEngine/Network/SocketUtils.h"

DeadlockDetector*       gDeadlockDetector = nullptr;
ThreadManager*          gThreadManager = nullptr;
MemoryPoolManager*      gMemoryPoolManager = nullptr;

GlobalContext::GlobalContext()
{
    gThreadManager      = new ThreadManager();
    gDeadlockDetector   = new DeadlockDetector();
    gMemoryPoolManager  = new MemoryPoolManager();
    SocketUtils::Init();
}

GlobalContext::~GlobalContext()
{
    SocketUtils::Cleanup();
    delete              gMemoryPoolManager;
    delete              gDeadlockDetector;
    delete              gThreadManager;
}

GlobalContext           gGlobalContext;
