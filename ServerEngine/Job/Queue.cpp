/*    ServerEngine/Job/Queue.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Job/Queue.h"

JobQueue::JobQueue()
    : mQueue(kInitQueueSize)
{}

void JobQueue::Push(SharedPtr<Job> job)
{
    // 큐에 job을 추가한다
    const Int64 prevCount = mJobCount.fetch_add(1);
    Bool result = mQueue.enqueue(std::move(job));
    ASSERT_CRASH_DEBUG(result == true, "ENQUEUE_FAILED");

    // job이 처음 들어왔을 때 큐를 매니저에 등록
    if (prevCount == 0)
    {
        gJobQueueManager->Register(shared_from_this());
    }
}

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

JobQueueManager::JobQueueManager()
    :mRunning(true)
{
    // 완료 포트 생성 (0은 시스템이 동시 스레드 수 결정)
    mIocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    ASSERT_CRASH(mIocp != nullptr, "CREATE_IOCP_FAILED");
}

JobQueueManager::~JobQueueManager()
{
    if (mIocp != nullptr)
    {
        ::CloseHandle(mIocp);
        mIocp = nullptr;
    }
}

void JobQueueManager::Register(SharedPtr<JobQueue> queue)
{
    // 등록 이벤트
    mEventCount.fetch_add(1);
    RegisterEvent* event = new RegisterEvent();
    event->Init();
    event->queue = std::move(queue);

    PostRegisterEvent(event);
}

void JobQueueManager::PostRegisterEvent(RegisterEvent* event)
{
    // 등록 이벤트 포스팅
    BOOL result = ::PostQueuedCompletionStatus(mIocp, 0, 0, event);
    if (result == FALSE)
    {
        HandleError(::GetLastError());
        delete event;
    }
}

void JobQueueManager::FlushQueues(UInt32 timeoutMs)
{
    while (mRunning)
    {
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey = 0;
        RegisterEvent* event = nullptr;

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
                delete event;
                continue;
            }
        }

        // JobQueue 플러시 시도
        Bool completed = event->queue->TryFlush(kFlushTimeoutMs);
        if (!completed)
        {
            // 모든 작업을 처리하지 못한 경우 다시 등록
            event->Init();
            PostRegisterEvent(event);
            continue;
        }

        // 이벤트 객체 메모리 해제
        delete event;
        mEventCount.fetch_sub(1);
    }
}

void JobQueueManager::HandleError(Int64 errorCode)
{
    gLogger->Error(TEXT_8("JobQueueManager: Error code: {}"), errorCode);
}
