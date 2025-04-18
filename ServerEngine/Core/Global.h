/*    ServerEngine/Core/Global.h    */

#pragma once

extern class ThreadManager*         gThreadManager;
extern class DeadlockDetector*      gDeadlockDetector;
extern class MemoryPoolManager*     gMemoryPoolManager;
extern class MemoryChunkPool*       gMemoryChunkPool;

class GlobalContext
{
public:
    GlobalContext();
    ~GlobalContext();
};

extern GlobalContext gGlobalContext;
