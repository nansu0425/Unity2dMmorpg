/*    ServerEngine/Core/Global.h    */

#pragma once

extern class ChunkMemoryPool*                   gChunkMemoryPool;
extern class ThreadManager*                     gThreadManager;
extern class DeadlockDetector*                  gDeadlockDetector;
extern class MemoryPoolManager*                 gMemoryPoolManager;
extern class GlobalBlockMemoryPoolManager*      gBlockMemoryPoolManager;

class GlobalContext
{
public:
    GlobalContext();
    ~GlobalContext();
};

extern GlobalContext gGlobalContext;
