/*    ServerEngine/Core/Global.h    */

#pragma once

extern class ThreadManager* gThreadManager;

class GlobalContext
{
public:
    GlobalContext();
    ~GlobalContext();
};

extern GlobalContext gGlobalContext;
