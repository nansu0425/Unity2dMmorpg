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
 * IOCP(I/O Completion Port)를 생성하고 초기화합니다.
 * mRunning을 true로 설정하여 작업 처리 루프가 활성화되도록 합니다.
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
 * IOCP 핸들이 존재하면 안전하게 해제합니다.
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
 * 1. 큐 카운트를 원자적으로 증가시킵니다.
 * 2. 큐를 IOCP에 등록하기 위해 Push 이벤트를 포스트합니다.
 */
void JobQueueManager::RegisterQueue(SharedPtr<JobQueue> queue)
{
    mQueueCount.fetch_add(1);

    // Push 이벤트 포스트
    PostPushEvent(std::move(queue));
}

/**
 * JobPushEvent를 IOCP에 포스트합니다.
 *
 * @param queue 이벤트를 포스트할 작업 큐
 *
 * 동작:
 * 1. 큐로부터 Push 이벤트를 가져옵니다.
 * 2. 이벤트를 초기화하고 큐의 소유권을 설정합니다.
 * 3. 이벤트를 IOCP에 포스트하여 작업 처리를 요청합니다.
 * 4. 포스트 실패 시 오류를 처리하고 큐 참조를 해제합니다.
 */
void JobQueueManager::PostPushEvent(SharedPtr<JobQueue> queue)
{
    JobPushEvent* event = queue->GetPushEvent();

    // 이벤트 초기화
    event->Init(std::move(queue));

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
 * @param timeoutMs IOCP 대기 타임아웃(밀리초), 기본값은 INFINITE
 *
 * 동작:
 * 1. mRunning이 true인 동안 계속 실행합니다.
 * 2. IOCP에서 완료 패킷을 기다립니다.
 * 3. 작업 종료 신호를 확인합니다.
 * 4. IOCP 대기 실패 시 적절히 오류를 처리합니다.
 * 5. 성공 시, 큐의 작업을 지정된 시간 동안 처리합니다.
 * 6. 모든 작업을 완료하지 못했으면 큐를 다시 등록합니다.
 * 7. 모든 작업을 완료했으면 큐 카운트를 감소시킵니다.
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
                mQueueCount.fetch_sub(1);
                continue;
            }
        }

        // JobQueue 플러시 시도
        SharedPtr<JobQueue> queue = std::move(event->owner);
        Bool completed = queue->TryFlush(kFlushTimeoutMs);
        if (!completed)
        {
            // 모든 작업을 처리하지 못한 경우 다시 이벤트 포스트
            PostPushEvent(std::move(queue));
            continue;
        }

        // JobQueue의 모든 작업을 처리한 경우
        mQueueCount.fetch_sub(1);
    }
}

/**
 * JobQueueManager 관련 오류를 처리합니다.
 *
 * @param errorCode 발생한 오류 코드
 *
 * 동작:
 * 글로벌 로거를 사용하여 오류 코드를 로그에 기록합니다.
 */
void JobQueueManager::HandleError(Int64 errorCode)
{
    gLogger->Error(TEXT_8("JobQueueManager: Error code: {}"), errorCode);
}
