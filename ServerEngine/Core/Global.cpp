/*    ServerEngine/Core/Global.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Concurrency/Deadlock.h"
#include "ServerEngine/Network/Socket.h"

ThreadManager*                  gThreadManager = nullptr;
DeadlockDetector*               gDeadlockDetector = nullptr;

GlobalContext::GlobalContext()
{
    gThreadManager              = new ThreadManager(); // TLS 영역 초기화
    gDeadlockDetector           = new DeadlockDetector();
    SocketUtils::Init();
}

GlobalContext::~GlobalContext()
{
    SocketUtils::Cleanup();
    delete              gDeadlockDetector;
    delete              gThreadManager;
}

GlobalContext*          gGlobalContext;
