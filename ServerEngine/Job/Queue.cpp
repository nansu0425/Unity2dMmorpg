/*    ServerEngine/Job/Queue.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Job/Queue.h"

/**
 * JobQueue 생성자
 * 락프리 큐를 지정된 초기 크기로 초기화합니다.
 */
JobQueue::JobQueue()
    : mQueue(kInitQueueSize)
{}

/**
 * 큐에 작업(Job)을 추가합니다.
 *
 * @param job 실행할 작업
 *
 * 동작:
 * 1. 먼저 작업 카운트를 원자적으로 증가시키고 이전 값을 저장합니다.
 * 2. 락프리 큐에 작업을 안전하게 추가합니다.
 * 3. 이전 작업 카운트가 0이었다면(첫 작업), JobQueueManager에 이 큐를 등록하여 작업 처리를 요청합니다.
 */
void JobQueue::Push(SharedPtr<Job> job)
{
    // 큐에 job을 추가한다
    const Int64 prevCount = mJobCount.fetch_add(1);
    Bool result = mQueue.enqueue(std::move(job));
    ASSERT_CRASH_DEBUG(result == true, "ENQUEUE_FAILED");

    // job이 처음 들어왔을 때 큐를 매니저에 등록
    if (prevCount == 0)
    {
        gJobQueueManager->RegisterQueue(shared_from_this());
    }
}

/**
 * 큐에 있는 작업들을 지정된 시간 내에 실행하려고 시도합니다.
 *
 * @param timeoutMs 작업 실행 최대 허용 시간(밀리초)
 * @return 모든 작업을 처리했으면 true, 시간 초과로 중단했으면 false
 *
 * 동작:
 * 1. 시작 시간을 기록하고 반복 실행을 시작합니다.
 * 2. 큐에서 작업을 하나 꺼내어 실행합니다.
 * 3. 작업 카운트를 감소시키고, 마지막 작업이면 종료합니다.
 * 4. 지정된 시간을 초과하면 false를 반환하고 중단합니다.
 */
Bool JobQueue::TryFlush(Int64 timeoutMs)
{
    const Int64 startTick = ::GetTickCount64();
    Bool ret = true;

    while (true)
    {
        // 실행할 job을 꺼낸다
        SharedPtr<Job> job;
        Bool result = mQueue.try_dequeue(job);
        ASSERT_CRASH_DEBUG(result == true, "TRY_DEQUEUE_FAILED");

        // job 실행
        job->Execute();

        // 모든 job을 처리한 경우 반복 종료
        if (mJobCount.fetch_sub(1) == 1)
        {
            break;
        }

        // 타임아웃 시간을 초과하면 반복 종료
        if (startTick + timeoutMs < static_cast<Int64>(::GetTickCount64()))
        {
            ret = false;
            break;
        }
    }

    return ret;
}

/**
 * JobQueueManager 생성자
 *
 * 동기화 객체를 초기화합니다.
 * - SRWLOCK(mLock)을 통해 스레드 간 동기화 보장
 * - CONDITION_VARIABLE(mCondVar)을 통해 효율적인 스레드 대기/깨우기 구현
 * - 큐를 지정된 초기 크기로 초기화
 */
JobQueueManager::JobQueueManager()
    : mQueues(kInitQueueSize)
{
    ::InitializeSRWLock(&mLock);
    ::InitializeConditionVariable(&mCondVar);
}

/**
 * JobQueue를 매니저에 등록합니다.
 *
 * @param queue 등록할 작업 큐
 *
 * 동작:
 * 1. 락프리 큐(mQueues)에 작업 큐를 안전하게 추가합니다.
 * 2. SRWLOCK을 사용한 배타적 락을 획득하여 스레드 안전성을 보장합니다.
 * 3. mWaked 플래그를 true로 설정하여 새 큐가 추가되었음을 표시합니다.
 * 4. 락을 해제한 후 WakeConditionVariable을 호출하여 대기 중인 스레드를 깨웁니다.
 */
void JobQueueManager::RegisterQueue(SharedPtr<JobQueue> queue)
{
    // 큐를 등록한다
    Bool result = mQueues.enqueue(std::move(queue));
    ASSERT_CRASH_DEBUG(result == true, "ENQUEUE_FAILED");

    {
        SrwLockWriteGuard gaurd(mLock);
        mWaked = true;
    }
    // 대기 스레드를 깨운다 
    ::WakeConditionVariable(&mCondVar);
}

/**
 * 등록된 큐들의 작업을 처리합니다.
 *
 * 동작:
 * 1. mRunning이 true인 동안 계속 실행합니다.
 * 2. 락프리 큐에서 작업 큐를 하나씩 꺼내어 처리합니다.
 * 3. 각 작업 큐의 작업을 지정된 시간(kFlushTimeoutMs) 동안 실행합니다.
 * 4. 모든 작업을 완료하지 못한 큐는 다시 등록하여 나중에 처리합니다.
 * 5. 모든 큐를 처리한 후 SRWLOCK을 사용하여 신호 상태를 확인합니다.
 * 6. 신호가 없으면(mWaked가 false) SleepConditionVariableSRW로 대기합니다.
 * 7. 깨어난 후에는 mWaked를 false로 초기화합니다.
 */
void JobQueueManager::FlushQueues()
{
    while (mRunning)
    {
        // 모든 큐를 비운다.
        SharedPtr<JobQueue> queue;
        while (mQueues.try_dequeue(queue))
        {
            // 큐의 작업을 지정된 시간 동안 처리한다.
            Bool completed = queue->TryFlush(kFlushTimeoutMs);
            if (!completed)
            {
                // 모든 작업을 처리하지 못한 경우 다시 등록한다.
                Bool result = mQueues.enqueue(std::move(queue));
                ASSERT_CRASH_DEBUG(result == true, "ENQUEUE_FAILED");
            }
        }

        // 배타적 잠금
        SrwLockWriteGuard guard(mLock);

        // 깨웠던 신호가 없으면 대기
        if (!mWaked)
        {
            BOOL result = ::SleepConditionVariableSRW(&mCondVar, &mLock, INFINITE, 0);
        }
        // 신호 초기화
        mWaked = false;
    }
}
