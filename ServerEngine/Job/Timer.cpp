/*    ServerEngine/Job/Timer.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Job/Timer.h"
#include <chrono>
#include <mutex>
#include <condition_variable>

/**
 * JobTimer 생성자
 * 조건 변수와 관련 멤버를 초기화합니다.
 */
JobTimer::JobTimer()
    : mNotified(false)
{}

/**
 * JobTimer 소멸자
 * 더 이상 명시적인 리소스 해제가 필요 없습니다.
 */
JobTimer::~JobTimer()
{
    // 조건 변수는 자동으로 정리됨
}

/**
 * 작업을 일정 시간 후에 실행되도록 스케줄링합니다.
 *
 * @param job 실행할 작업 객체
 * @param queue 작업이 실행될 큐에 대한 약한 참조
 * @param delayMs 실행 지연 시간(밀리초)
 *
 * 동작:
 * 1. 현재 시간에 지연 시간을 더해 실행 시간을 계산합니다.
 * 2. 작업, 큐, 실행 시간을 포함한 아이템을 우선순위 큐에 추가합니다.
 * 3. 타이머 스레드를 깨워 새 스케줄을 처리하도록 합니다.
 */
void JobTimer::Schedule(SharedPtr<Job> job, WeakPtr<JobQueue> queue, Int64 delayMs)
{
    const Int64 execTick = ::GetTickCount64() + delayMs;

    {
        WRITE_GUARD;
        mScheduledItems.push(Item{execTick, std::move(job), std::move(queue)});
    }

    // 타이머 스레드 깨우기 (조건 변수 사용)
    {
        std::lock_guard<std::mutex> lock(mWakeMutex);
        mNotified = true;
    }
    mWakeCV.notify_one();
}

/**
 * 실행 시간이 된 작업들을 해당 JobQueue로 분배합니다.
 *
 * @return 다음 작업 실행까지 대기해야 할 시간(밀리초)
 *
 * 동작:
 * 1. 현재 시간을 기준으로 실행 시간이 도래한 작업들을 우선순위 큐에서 꺼냅니다.
 * 2. 꺼낸 작업들을 각각의 JobQueue에 푸시합니다.
 * 3. 다음 실행 예정 작업까지의 대기 시간을 계산하여 반환합니다.
 * 4. 우선순위 큐가 비어있으면 최대 대기 시간(kMaxWaitMs)을 반환합니다.
 */
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

/**
 * 타이머 쓰레드의 메인 루프입니다.
 *
 * 동작:
 * 1. 이미 실행 중인지 확인하고, 실행 상태로 설정합니다.
 * 2. 지속적으로 Distribute()를 호출하여 작업을 분배합니다.
 * 3. 다음 작업까지 조건 변수로 대기하거나 타임아웃합니다.
 * 4. mRunning이 false가 될 때까지 반복합니다.
 */
void JobTimer::Run()
{
    ASSERT_CRASH(mRunning == false, "ALREADY_RUNNING");
    mRunning = true;

    while (mRunning)
    {
        // 타이머 설정 시간이 지난 잡을 큐에 분배
        Int64 waitMs = Distribute();

        // 조건 변수를 사용한 대기
        std::unique_lock<std::mutex> lock(mWakeMutex);
        if (waitMs > 0)
        {
            // 신호 도착 또는 타임아웃까지 대기
            mWakeCV.wait_for(lock,
                             std::chrono::milliseconds(waitMs),
                             [this]() { return mNotified; });

            // 신호를 처리했으므로 플래그 리셋
            mNotified = false;
        }
    }
}
