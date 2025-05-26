/*    ServerCore/Concurrency/Deadlock.h    */

#pragma once

/*
 * DeadlockDetector 클래스
 *
 * 목적:
 * - 실행 시간에 잠재적인 데드락 상황을 감지하는 도구
 * - 락 획득 순서를 추적하여 사이클이 발생하는 경우 데드락으로 판단
 *
 * 주요 기능:
 * - 재귀 잠금 방지 (동일 락을 중복 획득 시 크래시 발생)
 * - 락 획득 순서를 방향 그래프로 관리하여 데드락 가능성 검사
 * - 새로운 잠금 순서 발견 시 그래프 업데이트 및 사이클 검사 수행
 *
 * 동작 방식:
 * - 락은 클래스 타입 이름으로 구분됨
 * - 쓰기 락과 읽기 락을 구분하지 않음 (락 종류와 무관하게 동일 객체로 취급)
 * - DFS 알고리즘을 사용하여 사이클 탐지
 */
class DeadlockDetector
{
public:
    void            PushLock(const Char8* name);
    void            PopLock(const Char8* name);
    void            CheckCycle();

private:
    SRWLOCK                         mLock = SRWLOCK_INIT;
    HashMap<const Char8*, Int32>    mNameToId;
    HashMap<Int32, const Char8*>    mIdToName;
    HashMap<Int32, HashSet<Int32>>  mlockGraph;

private:
    void            Dfs(Int32 current);

    Vector<Int32>   mVisitHistory;
    Int32           mNextVisitOrder = 0;
    Vector<Bool>    mDfsFinished;
    Vector<Int32>   mParent;
};
