/*    ServerEngine/Job/Timer.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Job/Timer.h"

JobTimer::JobTimer()
{
    mWakeEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    ASSERT_CRASH(mWakeEvent != NULL, "CREATE_TIMER_EVENT_FAILED");
}

JobTimer::~JobTimer()
{
    if (mWakeEvent != NULL)
    {
        ::CloseHandle(mWakeEvent);
        mWakeEvent = NULL;
    }
}

void JobTimer::Schedule(SharedPtr<Job> job, WeakPtr<JobQueue> queue, Int64 delayMs)
{
    const Int64 execTick = ::GetTickCount64() + delayMs;

    {
        WRITE_GUARD;
        mScheduledItems.push(Item{execTick, std::move(job), std::move(queue)});
    }

    // 타이머 스레드 깨우기
    ::SetEvent(mWakeEvent);
}

Int64 JobTimer::Distribute()
{
    const Int64 nowTick = ::GetTickCount64();
    // 다음 실행 대기 시간
    Int64 waitMs = kMaxWaitMs;

    {
        WRITE_GUARD;

        // 실행 가능한 틱에 도달한 Item을 모두 꺼낸다
        while (!mScheduledItems.empty())
        {
            const Item& item = mScheduledItems.top();
            // 실행 틱에 도달하지 못한 경우
            if (nowTick < item.execTick)
            {
                waitMs = item.execTick - nowTick;  
                break;
            }

            mExecItems.emplace_back(item);
            mScheduledItems.pop();
        }
    }

    // 꺼낸 모든 아이템의 job을 큐에 넣는다
    for (Item& item : mExecItems)
    {
        item.queue.lock()->Push(std::move(item.job));
    }
    mExecItems.clear();

    return waitMs; 
}

void JobTimer::Run()
{
    ASSERT_CRASH(mRunning == false, "ALREADY_RUNNING");
    mRunning = true;

    while (mRunning)
    {
        // 타이머 설정 시간이 지난 잡을 큐에 분배
        Int64 waitMs = Distribute();
        // 이벤트 대기 또는 타임아웃
        ::WaitForSingleObject(mWakeEvent, static_cast<DWORD>(waitMs));
    }
}
