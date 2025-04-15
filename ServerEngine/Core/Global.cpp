/*    ServerEngine/Core/Global.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Concurrency/Deadlock.h"

DeadlockDetector*       gDeadlockDetector = nullptr;
ThreadManager*          gThreadManager = nullptr;

GlobalContext::GlobalContext()
{
    gThreadManager      = new ThreadManager();
    gDeadlockDetector   = new DeadlockDetector();
}

GlobalContext::~GlobalContext()
{
    delete              gDeadlockDetector;
    delete              gThreadManager;
}

GlobalContext           gGlobalContext;
