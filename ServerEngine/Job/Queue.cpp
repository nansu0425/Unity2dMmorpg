/*    ServerEngine/Job/Queue.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Job/Queue.h"
#include "ServerEngine/Job/Event.h"

/**
 * JobQueue 생성자
 * 락프리 큐를 초기 크기로 초기화합니다.
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
 * 1. 큐에 작업을 추가하고 작업 카운트를 증가시킵니다.
 * 2. 최초 작업 추가 시 JobQueueManager에 이 큐를 등록하여 작업 처리를 요청합니다.
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
 * IOCP(I/O Completion Port)를 생성하고 초기화합니다.
 */
JobQueueManager::JobQueueManager()
    :mRunning(true)
{
    // 완료 포트 생성
    mIocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    ASSERT_CRASH(mIocp != nullptr, "CREATE_IOCP_FAILED");
}

/**
 * JobQueueManager 소멸자
 * IOCP 핸들을 정리합니다.
 */
JobQueueManager::~JobQueueManager()
{
    if (mIocp != nullptr)
    {
        ::CloseHandle(mIocp);
        mIocp = nullptr;
    }
}

/**
 * JobQueue를 매니저에 등록합니다.
 *
 * @param queue 등록할 작업 큐
 */
void JobQueueManager::RegisterQueue(SharedPtr<JobQueue> queue)
{
    mQueueCount.fetch_add(1);

    // 새 이벤트 할당
    JobPushEvent* event = gJobPushEventPool->Pop();
    event->owner = std::move(queue);
    
    // Push 이벤트 포스트
    PostPushEvent(event);
}

/**
 * JobPushEvent를 IOCP에 포스트합니다.
 */
void JobQueueManager::PostPushEvent(JobPushEvent* event)
{
    // 이벤트 포스트
    BOOL result = ::PostQueuedCompletionStatus(mIocp, 0, 0, event);
    if (result == FALSE)
    {
        HandleError(::GetLastError());
        event->owner.reset();
        gJobPushEventPool->Push(event);
    }
}

/**
 * 등록된 큐들의 작업을 처리합니다.
 */
void JobQueueManager::FlushQueues(UInt32 timeoutMs)
{
    while (mRunning)
    {
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey = 0;
        JobPushEvent* event = nullptr;

        // IOCP에서 완료 패킷 대기
        BOOL success = ::GetQueuedCompletionStatus(
            mIocp,
            &bytesTransferred,
            &completionKey,
            (LPOVERLAPPED*)&event,
            timeoutMs
        );

        // 종료 체크
        if (mRunning == false)
            break;

        // 실패 처리
        if (!success)
        {
            const Int64 errorCode = ::GetLastError();

            if (event == nullptr)
            {
                // 함수 호출 자체 실패
                if (errorCode != WAIT_TIMEOUT)
                {
                    HandleError(errorCode);
                }
                return;
            }
            else
            {
                // 실패지만 event는 유효함
                HandleError(errorCode);
                event->owner.reset();
                gJobPushEventPool->Push(event);
                continue;
            }
        }

        // JobQueue 플러시 시도
        Bool completed = event->owner->TryFlush(kFlushTimeoutMs);
        if (!completed)
        {
            // 모든 작업을 처리하지 못한 경우 다시 이벤트 포스트
            event->Init();
            PostPushEvent(event);
            continue;
        }

        // JobQueue의 모든 작업을 처리한 경우
        event->owner.reset();
        gJobPushEventPool->Push(event);
        mQueueCount.fetch_sub(1);
    }
}

/**
 * JobQueueManager 관련 오류를 처리합니다.
 */
void JobQueueManager::HandleError(Int64 errorCode)
{
    gLogger->Error(TEXT_8("JobQueueManager: Error code: {}"), errorCode);
}
