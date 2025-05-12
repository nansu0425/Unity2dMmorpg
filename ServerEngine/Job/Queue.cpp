/*    ServerEngine/Job/Queue.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Job/Queue.h"

void JobQueue::Push(SharedPtr<Job> job, Bool canFlush)
{
    const Int64 prevCount = mJobCount.fetch_add(1);
    mJobs.Push(std::move(job));

    // 큐에 job을 처음 넣은 스레드만 진입
    if (prevCount == 0)
    {
        if (tFlushingQueue.expired() && // 현재 스레드가 비우고 있는 큐가 없는 경우
            (canFlush == true))
        {
            Flush();
        }
        else
        {
            // 큐 매니저의 FlushQueues()를 호출할 때 큐를 비운다
            gJobQueueManager->Register(shared_from_this());
        }
    }
}

void JobQueue::Flush()
{
    // 현재 스레드가 비우고 있는 큐로 설정
    tFlushingQueue = shared_from_this();

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
        // 처리한 job의 개수만큼 mJobCount를 감소
        if (mJobCount.fetch_sub(jobCount) == jobCount)
        {
            // 모든 job을 처리한 경우 반복 종료
            break;
        }
        // 워커 루프 시간을 초과했는지 확인
        if (tWorkerLoopTick < ::GetTickCount64())
        {
            gJobQueueManager->Register(shared_from_this());
            break;
        }
        jobs.clear();
    }

    // 비우고 있는 큐가 없는 상태로 설정
    tFlushingQueue.reset();
}

void JobQueueManager::Register(SharedPtr<JobQueue> queue)
{
    mQueues.Push(std::move(queue));
}

void JobQueueManager::FlushQueues()
{
    while (true)
    {
        // 워커 루프 시간을 초과했는지 확인
        if (tWorkerLoopTick < ::GetTickCount64())
        {
            break;
        }
        // 등록된 큐가 없으면 종료
        SharedPtr<JobQueue> queue = nullptr;
        if (mQueues.Pop(queue) == false)
        {
            break;
        }
        // 등록된 큐를 비운다
        queue->Flush();
    }
}
