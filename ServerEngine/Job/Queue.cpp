/*    ServerEngine/Job/Queue.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Job/Queue.h"

//void JobQueue::Push(SharedPtr<Job> job, Bool canFlush)
//{
//    const Int64 prevCount = mJobCount.fetch_add(1);
//    mJobs.Push(std::move(job));
//
//    // 큐에 job을 처음 넣은 스레드만 진입
//    if (prevCount == 0)
//    {
//        if (tFlushingQueue.expired() && // 현재 스레드가 비우고 있는 큐가 없는 경우
//            (canFlush == true))
//        {
//            Flush();
//        }
//        else
//        {
//            // 큐 매니저의 FlushQueues()를 호출할 때 큐를 비운다
//            gJobQueueManager->Register(shared_from_this());
//        }
//    }
//}

JobQueue::JobQueue()
    : mQueue(kQueueSize)
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

Bool JobQueue::Flush(UInt64 timeoutMs)
{
    const UInt64 start = ::GetTickCount64();
    Bool result = true;

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
        if (start + timeoutMs < ::GetTickCount64())
        {
            result = false;
            break;
        }
    }

    return result;
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
    // 등록 이벤트 생성 후 포스팅
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
                // 함수 호출 자체 실패(타임아웃 포함)
                if (errorCode != WAIT_TIMEOUT)
                {
                    HandleError(errorCode);
                }
                return;
            }
            else
            {
                // I/O 작업 실패지만 event는 유효함
                HandleError(errorCode);
                delete event;
                continue;
            }
        }

        // JobQueue 플러시 시도
        Bool completed = event->queue->Flush(kFlushTimeoutMs);
        if (!completed)
        {
            // 모든 작업을 처리하지 못한 경우 다시 큐에 등록
            event->Init();
            PostRegisterEvent(event);
            continue;
        }

        // 이벤트 객체 메모리 해제
        delete event;
    }
}

void JobQueueManager::HandleError(Int64 errorCode)
{
    gLogger->Error(TEXT_8("JobQueueManager: Error code: {}"), errorCode);
}

//void JobQueueManager::Register(SharedPtr<JobQueue> queue)
//{
//    mQueues.Push(std::move(queue));
//}
//
//void JobQueueManager::FlushQueues()
//{
//    while (true)
//    {
//        // 워커 루프 시간을 초과했는지 확인
//        if (tWorkerLoopTick < ::GetTickCount64())
//        {
//            break;
//        }
//        // 등록된 큐가 없으면 종료
//        SharedPtr<JobQueue> queue = nullptr;
//        if (mQueues.Pop(queue) == false)
//        {
//            break;
//        }
//        // 등록된 큐를 비운다
//        queue->Flush();
//    }
//}
