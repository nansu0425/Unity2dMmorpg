/*    ServerEngine/Job/Timer.h    */

#pragma once

/*
 * JobTimer는 일정 시간 후에 JobQueue에 Job이 Push되도록 스케줄링한다.
 * Distribute()를 호출하면 스케줄링된 Job이 JobQueue에 Push된다.
 */
class JobTimer
{
public:
    struct Item
    {
        Int64               executeTick;
        SharedPtr<Job>      job;
        WeakPtr<JobQueue>   owner;

        bool    operator<(const Item& other) const { return (executeTick > other.executeTick); }
    };

public:
    void        Schedule(SharedPtr<Job> job, WeakPtr<JobQueue> owner, Int64 delayTick);
    void        Distribute();

private:
    RW_LOCK;
    PriorityQueue<Item>     mItems;
    Atomic<Bool>            mIsDistributing = false;
};
