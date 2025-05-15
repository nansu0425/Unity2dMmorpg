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
        Int64               execTick;
        SharedPtr<Job>      job;
        WeakPtr<JobQueue>   queue;

        bool    operator<(const Item& other) const { return (execTick > other.execTick); }
    };

public:
    JobTimer();
    ~JobTimer();

    void        Schedule(SharedPtr<Job> job, WeakPtr<JobQueue> queue, Int64 delayMs);
    Int64       Distribute();
    void        Run();

private:
    RW_LOCK;
    PriorityQueue<Item>     mScheduledItems;
    Bool                    mRunning = false;
    HANDLE                  mWakeEvent = NULL;
    Vector<Item>            mExecItems;

    static constexpr Int64  kMaxWaitMs = 100;
};
