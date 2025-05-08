/*    ServerEngine/Common/Global.h    */

#pragma once

extern class Logger*                gLogger;
extern class ThreadManager*         gThreadManager;
extern class DeadlockDetector*      gDeadlockDetector;
extern class JobQueueManager*       gJobQueueManager;
extern class JobTimer*              gJobTimer;

class GlobalContext
{
public:
    GlobalContext();
    ~GlobalContext();
};

extern GlobalContext    gGlobalContext;
