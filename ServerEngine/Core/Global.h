/*    ServerEngine/Core/Global.h    */

#pragma once

extern class ThreadManager*     gThreadManager;
extern class DeadlockDetector*  gDeadlockDetector;
extern class Logger*            gLogger;

class GlobalContext
{
public:
    GlobalContext();
    ~GlobalContext();
};

extern GlobalContext    gGlobalContext;
