/*    ServerEngine/Concurrency/Deadlock.h    */

#pragma once

class DeadlockDetector
{
public:
    void    PushLock(const Char8* name);
    void    PopLock(const Char8* name);
    void    CheckCycle();

private:
    SharedMutex     mLock;

    HashMap<const Char8*, Int32>    mNameToId;
    HashMap<Int32, const Char8*>    mIdToName;
    HashMap<Int32, HashSet<Int32>>  mlockGraph;

private:
    void    Dfs(Int32 current);

    Vector<Int32>   mVisitHistory;
    Int32           mNextVisitOrder = 0;
    Vector<Bool>    mDfsFinished;
    Vector<Int32>   mParent;
};
