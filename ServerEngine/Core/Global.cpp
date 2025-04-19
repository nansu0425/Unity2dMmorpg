/*    ServerEngine/Core/Global.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Concurrency/Deadlock.h"
#include "ServerEngine/Memory/MemoryPool.h"
#include "ServerEngine/Network/SocketUtils.h"

ChunkMemoryPool*        gChunkMemoryPool = nullptr;
DeadlockDetector*       gDeadlockDetector = nullptr;
ThreadManager*          gThreadManager = nullptr;
MemoryPoolManager*      gMemoryPoolManager = nullptr;

GlobalContext::GlobalContext()
{
    gChunkMemoryPool    = new ChunkMemoryPool();
    gThreadManager      = new ThreadManager(); // TLS 영역 초기화
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
    delete              gChunkMemoryPool;
}

GlobalContext           gGlobalContext;
