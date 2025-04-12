/*    ServerEngine/Core/Global.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"

ThreadManager* gThreadManager = nullptr;

GlobalContext::GlobalContext()
{
    gThreadManager = new ThreadManager();
}

GlobalContext::~GlobalContext()
{
    delete gThreadManager;
    gThreadManager = nullptr;
}

GlobalContext gGlobalContext;
