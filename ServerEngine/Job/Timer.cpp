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

void JobTimer::Schedule(SharedPtr<Job> job, SharedPtr<JobQueue> owner, Int64 delayTick)
{
    const Int64 executeTick = ::GetTickCount64() + delayTick;

    {
        WRITE_GUARD;
        mItems.push(Item{executeTick, std::move(job), std::move(owner)});
    }

    // 타이머 스레드 깨우기
    ::SetEvent(mWakeEvent);
}

void JobTimer::Distribute()
{
    const Int64 nowTick = ::GetTickCount64();
    Vector<Item> executeItems;

    {
        WRITE_GUARD;
        // 실행 가능한 틱에 도달한 Item을 모두 꺼낸다
        while (!mItems.empty())
        {
            const Item& item = mItems.top();
            if (nowTick < item.executeTick)
            {
                break;
            }
            executeItems.push_back(item);
            mItems.pop();
        }
    }

    // 꺼낸 모든 아이템의 job을 큐에 넣는다
    for (const Item& item : executeItems)
    {
        item.owner->Push(item.job);
    }
}

Int64 JobTimer::GetTimeUntilNextItem()
{
    WRITE_GUARD;

    if (mItems.empty())
    {
        return kMaxWaitTime;
    }

    const Int64 nowTick = ::GetTickCount64();
    const Int64 nextTick = mItems.top().executeTick;

    if (nextTick <= nowTick)
    {
        return 0;  // 즉시 실행
    }

    return nextTick - nowTick;  // 다음 타이머까지 대기 시간
}

void JobTimer::Run()
{
    ASSERT_CRASH(mRunning == false, "ALREADY_RUNNING");
    mRunning = true;

    while (mRunning)
    {
        // 타이머 설정 시간이 지난 잡을 큐에 분배
        Distribute();

        // 다음 타이머 실행 시간까지 대기
        Int64 waitTime = GetTimeUntilNextItem();
        if (waitTime > 0)
        {
            // 이벤트 대기 또는 타임아웃
            ::WaitForSingleObject(mWakeEvent, static_cast<DWORD>(waitTime));
        }
    }
}
