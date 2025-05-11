/*    ServerEngine/Job/Timer.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Job/Timer.h"

void JobTimer::Schedule(SharedPtr<Job> job, WeakPtr<JobQueue> owner, Int64 delayTick)
{
    const Int64 executeTick = ::GetTickCount64() + delayTick;

    WRITE_GUARD;
    mItems.push(Item{executeTick, std::move(job), std::move(owner)});
}

void JobTimer::Distribute()
{
    // 중복 실행 방지
    if (mIsDistributing.exchange(true))
    {
        return;
    }

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
        SharedPtr<JobQueue> owner = item.owner.lock();
        if (owner)
        {
            owner->Push(item.job);
        }
    }

    // 분배 종료 처리
    mIsDistributing.store(false);
}
