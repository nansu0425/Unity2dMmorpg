/*    ServerEngine/Core/Global.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Concurrency/Deadlock.h"

ThreadManager*          gThreadManager = nullptr;
DeadlockDetector*       gDeadlockDetector = nullptr;

GlobalContext::GlobalContext()
{
    gThreadManager      = new ThreadManager();
    gDeadlockDetector   = new DeadlockDetector();
}

GlobalContext::~GlobalContext()
{
    delete  gDeadlockDetector;
    gDeadlockDetector   = nullptr;
    delete  gThreadManager;
    gThreadManager      = nullptr;
}

GlobalContext gGlobalContext;
