/*    ServerEngine/Concurrency/Deadlock.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Concurrency/Deadlock.h"

void DeadlockDetector::PushLock(const Char8* name)
{
    LockGuard guard(mLock);

    Int32 lockId = 0;
    auto nameIter = mNameToId.find(name);
    
    if (nameIter == mNameToId.end())
    {
        // 처음 확인된 락인 경우
        lockId = static_cast<Int32>(mNameToId.size());
        mNameToId[name] = lockId;
        mIdToName[lockId] = name;
    }
    else
    {
        // 락 id를 가져온다
        lockId = nameIter->second;
    }

    // 이미 잠근 락이 있는 경우
    if (!tLockStack.empty())
    {
        const Int32 prevLockId = tLockStack.top();
        // 같은 락을 다시 잠그는 경우 크래시
        if (prevLockId == lockId)
        {
            CRASH("MULTIPLE_LOCK");
        }
        HashSet<Int32>& nextLocks = mlockGraph[prevLockId];
        // 새로 발견한 잠금 순서라면 데드락 여부 확인
        if (nextLocks.find(lockId) == nextLocks.end())
        {
            nextLocks.insert(lockId);
            CheckCycle();
        }
    }

    tLockStack.push(lockId);
}

void DeadlockDetector::PopLock(const Char8* name)
{
    LockGuard guard(mLock);

    if (tLockStack.empty())
    {
        CRASH("MULTIPLE_UNLOCK");
    }

    Int32 lockId = mNameToId[name];
    if (tLockStack.top() != lockId)
    {
        CRASH("INVALID_UNLOCK_ORDER");
    }

    tLockStack.pop();
}

void DeadlockDetector::CheckCycle()
{
    const Int32 lockCount = static_cast<Int32>(mNameToId.size());
    mVisitHistory.assign(lockCount, -1);
    mNextVisitOrder = 0;
    mDfsFinished.assign(lockCount, false);
    mParent.assign(lockCount, -1);

    // 락 그래프의 모든 정점에 대해 Dfs를 수행
    for (Int32 i = 0; i < lockCount; ++i)
    {
        if (!mDfsFinished[i])
        {
            Dfs(i);
        }
    }

    mParent.clear();
    mDfsFinished.clear();
    mVisitHistory.clear();
}

void DeadlockDetector::Dfs(Int32 current)
{
    // 방문한 적 없는 락만 Dfs 수행
    ASSERT_CRASH(mVisitHistory[current] == -1, "MULTIPLE_DFS");

    mVisitHistory[current] = mNextVisitOrder++;
    auto currentIter = mlockGraph.find(current);
    // 다음 잠금이 없는 경우
    if (currentIter == mlockGraph.end())
    {
        mDfsFinished[current] = true;
        return;
    }

    HashSet<Int32>& nextLocks = currentIter->second;
    for (Int32 next : nextLocks)
    {
        // 아직 방문한 적 없는 락에 대해 Dfs
        if (mVisitHistory[next] == -1)
        {
            mParent[next] = current;
            Dfs(next);
            continue;
        }
        // current을 next보다 먼저 방문한 경우 순방향 간선
        if (mVisitHistory[current] < mVisitHistory[next])
        {
            continue;
        }
        // 순방향 간선이 아니고, next의 Dfs가 끝나지 않은 경우 역방향 간선 => 데드락
        if (!mDfsFinished[next])
        {
            // 사이클 경로 출력
            std::cout << "Deadlock detected: " << mIdToName[next];
            while (true)
            {
                std::cout << " <- " << mIdToName[current];
                if (current == next)
                {
                    break;
                }
                current = mParent[current];
            }
            std::cout << std::endl;

            CRASH("DEADLOCK_DETECTED");
        }
    }

    mDfsFinished[current] = true;
}
