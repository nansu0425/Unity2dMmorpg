/*    ServerEngine/Job/Timer.h    */

#pragma once

class JobTimer
{
public:
    struct TimerItem
    {
        Int64               executeTick;
        SharedPtr<Job>      job;
        WeakPtr<JobQueue>   jobOwner;

        bool        operator<(const TimerItem& other) const { return executeTick > other.executeTick; }
    };

public:
    void    Schedule(SharedPtr<Job> job, WeakPtr<JobQueue> owner, Int64 delayTick);
    void    Distribute(Int64 nowTick);

private:
    RW_LOCK;
    RankQueue<TimerItem>    mReservedItems;
    Atomic<Bool>            mIsDistributing = false;
};
