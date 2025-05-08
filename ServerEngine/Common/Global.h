/*    ServerEngine/Common/Global.h    */

#pragma once

extern class Logger*                gLogger;
extern class ThreadManager*         gThreadManager;
extern class DeadlockDetector*      gDeadlockDetector;
extern class ReservedJobManager*    gReservedJobManager;
extern class JobTimer*              gJobTimer;

class GlobalContext
{
public:
    GlobalContext();
    ~GlobalContext();
};

extern GlobalContext    gGlobalContext;
