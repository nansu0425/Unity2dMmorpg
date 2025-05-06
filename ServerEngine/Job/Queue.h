/*    ServerEngine/Job/Queue.h    */

#pragma once

class Job;

class JobQueue
{
public:
    void Push(SharedPtr<Job> job)
    {
        WRITE_GUARD;
        mJobs.push(std::move(job));
    }

    SharedPtr<Job> Pop()
    {
        SharedPtr<Job> job = nullptr;

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
    Queue<SharedPtr<Job>>  mJobs;
};
