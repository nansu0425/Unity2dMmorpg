/*    ServerEngine/Core/Global.h    */

#pragma once

extern class Logger*                gLogger;
extern class ThreadManager*         gThreadManager;
extern class DeadlockDetector*      gDeadlockDetector;
extern class ReservedJobsManager*   gReservedJobsManager;

class GlobalContext
{
public:
    GlobalContext();
    ~GlobalContext();
};

extern GlobalContext    gGlobalContext;
