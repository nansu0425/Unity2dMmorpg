/*    ServerEngine/Core/GlobalContext.h    */

#pragma once

extern class ThreadManager* g_threadManager;

class GlobalContext
{
public:
    GlobalContext();
    ~GlobalContext();
};

extern GlobalContext g_globalContext;
