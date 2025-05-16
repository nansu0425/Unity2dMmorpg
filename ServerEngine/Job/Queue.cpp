/*    ServerEngine/Job/Queue.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Job/Queue.h"

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
 * 2. 최초 작업 추가 시(prevCount == 0) PushEvent를 초기화하고
 *    JobQueueManager에 이 큐를 등록하여 작업 처리를 요청합니다.
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
        mPushEvent.Init();
        mPushEvent.owner = shared_from_this();
        gJobQueueManager->RegisterQueue(shared_from_this());
    }
}

/**
 * 큐에 있는 작업들을 지정된 시간 내에 실행하려고 시도합니다.
 *
 * @param timeoutMs 최대 실행 시간(밀리초)
 * @return 모든 작업 처리 성공 여부
 *
 * 동작:
 * 1. 큐에서 작업을 꺼내서 실행합니다.
 * 2. 작업 카운트를 감소시키고, 카운트가 0이 되면 모든 작업이 처리된 것으로 간주합니다.
 * 3. 타임아웃이 발생하면 처리를 중단하고 false를 반환합니다.
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
 *
 * 동작:
 * 1. 실행 상태를 true로 설정합니다.
 * 2. IOCP를 생성하여 비동기 작업 처리의 기반을 마련합니다.
 * 3. 마지막 매개변수(0)는 시스템이 동시 스레드 수를 자동으로 결정하게 합니다.
 */
JobQueueManager::JobQueueManager()
    :mRunning(true)
{
    // 완료 포트 생성 (0은 시스템이 동시 스레드 수 결정)
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
 *
 * 동작:
 * 1. PushEvent의 owner가 유효한지 확인합니다.
 * 2. 이벤트 카운트를 증가시킵니다. 
 * 3. PushEvent를 IOCP에 등록하여 작업 처리를 요청합니다.
 */
void JobQueueManager::RegisterQueue(SharedPtr<JobQueue> queue)
{
    ASSERT_CRASH_DEBUG(queue->GetPushEvent()->owner == queue, "INVALID_EVENT_OWNER");

    // Push 이벤트 포스트
    mEventCount.fetch_add(1);
    PostPushEvent(queue->GetPushEvent());
}

/**
 * PushEvent를 IOCP에 등록합니다.
 *
 * @param event 등록할 이벤트
 *
 * 동작:
 * 1. 이벤트를 IOCP에 포스트합니다.
 * 2. 실패 시 오류를 처리하고 이벤트 소유권을 해제합니다.
 */
void JobQueueManager::PostPushEvent(JobQueue::PushEvent* event)
{
    // 이벤트 포스트
    BOOL result = ::PostQueuedCompletionStatus(mIocp, 0, 0, event);
    if (result == FALSE)
    {
        HandleError(::GetLastError());
        event->owner.reset();
    }
}

/**
 * 등록된 큐들의 작업을 처리합니다.
 *
 * @param timeoutMs IOCP 대기 타임아웃(밀리초)
 *
 * 동작:
 * 1. IOCP에서 완료 패킷을 대기합니다.
 * 2. 패킷을 받으면 해당 큐의 작업을 TryFlush로 처리합니다.
 * 3. 모든 작업 처리 성공 시 이벤트 소유권을 해제하고 카운트를 감소시킵니다.
 * 4. 실패 시 이벤트를 다시 포스트하여 나중에 다시 처리합니다.
 */
void JobQueueManager::FlushQueues(UInt32 timeoutMs)
{
    while (mRunning)
    {
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey = 0;
        JobQueue::PushEvent* event = nullptr;

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
        mEventCount.fetch_sub(1);
    }
}

/**
 * JobQueueManager 관련 오류를 처리합니다.
 *
 * @param errorCode Windows 오류 코드
 *
 * 동작:
 * 로거를 통해 오류 코드를 기록합니다.
 */
void JobQueueManager::HandleError(Int64 errorCode)
{
    gLogger->Error(TEXT_8("JobQueueManager: Error code: {}"), errorCode);
}
