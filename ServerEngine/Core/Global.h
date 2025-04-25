/*    ServerEngine/Core/Global.h    */

#pragma once

extern class ThreadManager*     gThreadManager;
extern class DeadlockDetector*  gDeadlockDetector;

class GlobalContext
{
public:
    GlobalContext();
    ~GlobalContext();
};

extern GlobalContext*   gGlobalContext;
