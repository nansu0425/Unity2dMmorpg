/*    ServerEngine/Job/Timer.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Job/Timer.h"

void JobTimer::Schedule(SharedPtr<Job> job, WeakPtr<JobQueue> owner, Int64 delayTick)
{
    const Int64 executeTick = ::GetTickCount64() + delayTick;

    WRITE_GUARD;
    mReservedItems.push(TimerItem{executeTick, std::move(job), std::move(owner)});
}

void JobTimer::Distribute(Int64 nowTick)
{
    // 중복 실행 방지
    if (mIsDistributing.exchange(true))
    {
        return;
    }

    Vector<TimerItem> executeItems;
    {
        WRITE_GUARD;
        // 실행해야 할 TimerItem을 모두 꺼낸다
        while (!mReservedItems.empty())
        {
            const TimerItem& reservedItem = mReservedItems.top();
            // 현재 틱이 아직 실행할 틱에 도달하지 않은 경우
            if (nowTick < reservedItem.executeTick)
            {
                break;
            }
            executeItems.push_back(reservedItem);
            mReservedItems.pop();
        }
    }

    // 꺼낸 TimerItem의 Job을 JobQueue에 Push한다 
    for (const TimerItem& executeItem : executeItems)
    {
        SharedPtr<JobQueue> jobOwner = executeItem.jobOwner.lock();
        if (jobOwner)
        {
            // 큐의 flush는 하지 않는다
            jobOwner->Push(executeItem.job, false);
        }
    }

    // 분배 종료 처리
    mIsDistributing.store(false);
}
