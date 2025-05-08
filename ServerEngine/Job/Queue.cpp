/*    ServerEngine/Job/Queue.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Job/Queue.h"

void JobQueue::Push(SharedPtr<Job> job)
{
    const Int64 prevCount = mJobCount.fetch_add(1);
    mJobs.Push(std::move(job));

    // 최초 job을 넣은 스레드가 진입
    if (prevCount == 0)
    {
        if (tReservedJobs.expired())
        {
            // 스레드에 예약된 job이 없으면 현재 큐의 job을 처리한다
            Flush();
        }
        else
        {
            // 예약된 jobs가 없는 스레드가 처리할 수 있도록 예약한다
            gReservedJobsManager->ReserveJobs(shared_from_this());
        }
    }
}

void JobQueue::Flush()
{
    // 큐의 jobs을 현재 스레드에 예약한다
    tReservedJobs = shared_from_this();

    Vector<SharedPtr<Job>> jobs;
    while (true)
    {
        // 모든 job을 꺼낸 후 실행
        mJobs.PopAll(jobs);
        Int64 jobCount = 0;
        for (auto& job : jobs)
        {
            job->Execute();
            ++jobCount;
        }
        // 큐에 남은 job이 없으면 종료
        if (mJobCount.fetch_sub(jobCount) == jobCount)
        {
            break;
        }
        // 워커 루프 시간을 초과하면 다음 루프에 job을 처리할 수 있도록 예약한다
        const Int64 now = ::GetTickCount64();
        if (now > tWorkerLoopTick)
        {
            gReservedJobsManager->ReserveJobs(shared_from_this());
            break;
        }
        jobs.clear();
    }

    // 예약된 jobs가 없는 것으로 처리
    tReservedJobs.reset();
}

void ReservedJobsManager::ReserveJobs(SharedPtr<JobQueue> jobs)
{
    mJobs.Push(std::move(jobs));
}

void ReservedJobsManager::ProcessJobs()
{
    while (true)
    {
        // 워커 루프 시간을 초과하면 종료
        Int64 now = ::GetTickCount64();
        if (now > tWorkerLoopTick)
        {
            break;
        }
        // 예약된 jobs가 없으면 종료
        SharedPtr<JobQueue> jobs = nullptr;
        if (false == mJobs.Pop(jobs))
        {
            break;
        }
        // 예약된 jobs를 처리
        jobs->Flush();
    }
}
