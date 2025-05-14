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
        Int64                   executeTick;
        SharedPtr<Job>          job;
        SharedPtr<JobQueue>     owner;

        bool    operator<(const Item& other) const { return (executeTick > other.executeTick); }
    };

public:
    JobTimer();
    ~JobTimer();

    void        Schedule(SharedPtr<Job> job, SharedPtr<JobQueue> owner, Int64 delayTick);
    void        Distribute();
    Int64       GetTimeUntilNextItem();
    void        Run();

private:
    RW_LOCK;
    PriorityQueue<Item>     mItems;
    Bool                    mRunning = false;
    HANDLE                  mWakeEvent = NULL;

    static constexpr Int64  kMaxWaitTime = 100;
};
