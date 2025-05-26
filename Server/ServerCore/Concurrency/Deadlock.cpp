/*    ServerCore/Concurrency/Deadlock.cpp    */

#include "ServerCore/Pch.h"
#include "ServerCore/Concurrency/Deadlock.h"

/*
 * 락 획득 시 호출되는 메서드
 *
 * 락을 획득할 때마다 락 그래프를 갱신하고 데드락 가능성을 검사합니다.
 * 새로운 락인 경우 ID를 할당하고, 기존 락 획득 순서와 다른 새로운 순서가
 * 발견되면 데드락 검사를 수행합니다.
 * 동일한 락을 중복해서 획득하려는 시도를 감지하여 방지합니다.
 *
 * @param name 락의 식별자 (보통 클래스 타입 이름)
 */
void DeadlockDetector::PushLock(const Char8* name)
{
    SrwLockWriteGuard guard(mLock);

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
        // 같은 락을 다시 잠그는 경우 크래시
        const Int32 prevLockId = tLockStack.top();
        ASSERT_CRASH(prevLockId != lockId, "MULTIPLE_LOCK");

        // 새로 발견한 잠금 순서라면 데드락 여부 확인
        HashSet<Int32>& nextLocks = mlockGraph[prevLockId];
        if (nextLocks.find(lockId) == nextLocks.end())
        {
            nextLocks.insert(lockId);
            CheckCycle();
        }
    }

    tLockStack.push(lockId);
}

/*
 * 락 해제 시 호출되는 메서드
 *
 * 락 스택에서 가장 최근에 획득한 락을 제거합니다.
 * 비정상적인 락 해제 패턴(빈 스택에서의 해제 시도, 잘못된 순서의 해제)을
 * 검증하여 프로그램 안정성을 보장합니다.
 *
 * @param name 해제할 락의 식별자
 */
void DeadlockDetector::PopLock(const Char8* name)
{
    SrwLockWriteGuard guard(mLock);

    ASSERT_CRASH(!tLockStack.empty(), "MULTIPLE_UNLOCK");

    Int32 lockId = mNameToId[name];
    ASSERT_CRASH(tLockStack.top() == lockId, "INVALID_UNLOCK_ORDER");
    tLockStack.pop();
}

/*
 * 락 그래프에서 사이클을 검사하는 메서드
 *
 * 락 그래프의 모든 정점에 대해 DFS를 수행하여 사이클(데드락 가능성)을 검사합니다.
 * 각 정점별로 방문 여부와 순서를 추적하고, 사이클 발견 시 경로를 추적할 수 있도록
 * 부모 정점 정보를 관리합니다.
 */
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

/*
 * DFS 알고리즘을 수행하는 메서드
 *
 * 현재 정점에서 시작해 DFS 알고리즘으로 그래프를 순회하며 역방향 간선을 탐지합니다.
 * 역방향 간선은 사이클이 존재한다는 의미이므로, 발견 시 데드락으로 판단합니다.
 * 사이클 발견 시 경로를 로그로 남기고 크래시를 발생시킵니다.
 *
 * @param current 현재 탐색 중인 락 ID
 */
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
            // 사이클 경로 로그
            String8 log = TEXT_8("Deadlock detected: ");
            log += mIdToName[next];
            while (true)
            {
                log += " <- ";
                log += mIdToName[current];
                if (current == next)
                {
                    break;
                }
                current = mParent[current];
            }

            gLogger->Critical(log);
            CRASH("DEADLOCK_DETECTED");
        }
    }

    mDfsFinished[current] = true;
}
