/*    ServerEngine/Concurrency/Deadlock.h    */

#pragma once

/*
 * 재귀 잠금 제한
 * 새로운 잠금 순서가 발견될 때 그래프을 업데이트 후 사이클 검사
 * 락은 클래스 이름으로 구분
 * 디버그 모드에서만 동작
 * 쓰기/읽기 락을 구분하지 않음
 */
class DeadlockDetector
{
private:
    template<typename K, typename V>
    using HashMap   = std::unordered_map<K, V>;
    template<typename K>
    using HashSet   = std::unordered_set<K>;
    template<typename T>
    using Vector    = std::vector<T>;

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
