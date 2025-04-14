/*    ServerEngine/Core/Global.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Concurrency/Deadlock.h"
#include "ServerEngine/Concurrency/Thread.h"

DeadlockDetector*       gDeadlockDetector = nullptr;
ThreadManager*          gThreadManager = nullptr;

GlobalContext::GlobalContext()
{
    gDeadlockDetector   = new DeadlockDetector();
    gThreadManager      = new ThreadManager();
}

GlobalContext::~GlobalContext()
{
    delete              gThreadManager;
    gThreadManager      = nullptr;
    delete              gDeadlockDetector;
    gDeadlockDetector   = nullptr;
}

GlobalContext           gGlobalContext;
