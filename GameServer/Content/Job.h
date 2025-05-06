/*    GameServer/Content/Job.h    */

#pragma once

class IJob
{
public:
    virtual void Execute() = 0;
};

class HealJob
    : public IJob
{
public:
    virtual void Execute() override
    {
        gLogger->Info(TEXT_8("{}에게 {}만큼 힐"), mTarget, mHealValue);
    }

    void    SetTarget(Int64 target) { mTarget = target; }
    void    SetHealValue(Int64 healValue) { mHealValue = healValue; }

private:
    Int64   mTarget = 0;
    Int64   mHealValue = 0;
};

class JobQueue
{
public:
    JobQueue() = default;
    ~JobQueue() = default;

    void Push(SharedPtr<IJob> job)
    {
        WRITE_GUARD;
        mJobs.push(std::move(job));
    }

    SharedPtr<IJob> Pop()
    {
        SharedPtr<IJob> job = nullptr;

        WRITE_GUARD;
        if (false == mJobs.empty())
        {
            job = mJobs.front();
            mJobs.pop();
        }

        return job;
    }

private:
    RW_LOCK;
    Queue<SharedPtr<IJob>>  mJobs;
};
