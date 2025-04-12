/*    ServerEngine/Core/GlobalContext.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Core/Thread.h"

ThreadManager* g_threadManager = nullptr;

GlobalContext::GlobalContext()
{
    g_threadManager = new ThreadManager();
}

GlobalContext::~GlobalContext()
{
    delete g_threadManager;
    g_threadManager = nullptr;
}

GlobalContext g_globalContext;
