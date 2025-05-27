/*    ServerCore/Job/Timer.cpp    */

#include "ServerCore/Pch.h"
#include "ServerCore/Job/Timer.h"

namespace core
{
    /**
     * JobTimer 생성자
     *
     * Windows 네이티브 동기화 객체를 초기화합니다.
     * - SRWLOCK(mLock)을 통해 스레드 간 동기화 보장
     * - CONDITION_VARIABLE(mWakeCV)을 통해 효율적인 스레드 대기/깨우기 구현
     * - 알림 상태 플래그(mNotified) 초기값 설정
     */
    JobTimer::JobTimer()
        : mWaked(false)
    {
        ::InitializeSRWLock(&mLock);
        ::InitializeConditionVariable(&mCondVar);
    }

    /**
     * JobTimer 소멸자
     *
     * Windows 네이티브 동기화 객체는 명시적 해제가 필요 없으므로 추가 정리 작업이 없습니다.
     */
    JobTimer::~JobTimer()
    {}

    /**
     * 작업을 일정 시간 후에 실행되도록 스케줄링합니다.
     *
     * @param job 실행할 작업 객체(Job 클래스의 공유 포인터)
     * @param queue 작업이 실행될 큐에 대한 약한 참조(WeakPtr)
     * @param delayMs 실행 지연 시간(밀리초)
     *
     * 동작:
     * 1. 현재 시간에 지연 시간을 더해 실행 시간을 계산합니다.
     * 2. SRWLOCK을 사용한 배타적 락을 획득하여 스레드 안전성을 보장합니다.
     * 3. 계산된 실행 시간과 함께 작업과 큐를 우선순위 큐에 추가합니다.
     * 4. mWaked 플래그를 true로 설정하여 새 작업이 추가되었음을 표시합니다.
     * 5. 락을 해제한 후 WakeConditionVariable을 호출하여 타이머 스레드를 깨웁니다.
     */
    void JobTimer::Schedule(SharedPtr<Job> job, WeakPtr<JobQueue> queue, Int64 delayMs)
    {
        const Int64 execTick = ::GetTickCount64() + delayMs;

        {
            // 배타적 락 획득
            SrwLockWriteGuard guard(mLock);
            mScheduledItems.push(Item{execTick, std::move(job), std::move(queue)});

            // 알림 플래그 설정
            mWaked = true;
        }
        ::WakeConditionVariable(&mCondVar);
    }

    /**
     * 실행 시간이 된 작업들을 해당 JobQueue로 분배합니다.
     *
     * @return Int64 다음 작업 실행까지 대기해야 할 시간(밀리초)
     *
     * 동작:
     * 1. 현재 시스템 시간을 기준으로 대기 시간을 계산합니다.
     * 2. SRWLOCK을 사용한 배타적 락을 획득하여 스레드 안전성을 보장합니다.
     * 3. 실행 시간에 도달한 작업들을 우선순위 큐에서 추출합니다.
     * 4. 실행 시간에 도달하지 않은 첫 작업까지의 대기 시간을 계산합니다.
     * 5. 락을 해제하고 추출한 작업들을 각각의 JobQueue에 푸시합니다.
     * 6. 다음 실행까지 대기 시간 또는 최대 대기 시간(kMaxWaitMs)을 반환합니다.
     */
    Int64 JobTimer::Distribute()
    {
        Int64 waitMs = kMaxWaitMs;

        {
            // mScheduledItems 접근을 위한 배타적 락 획득
            SrwLockWriteGuard guard(mLock);

            // 실행 가능한 틱에 도달한 Item을 모두 꺼냄
            while (!mScheduledItems.empty())
            {
                const Item& item = mScheduledItems.top();
                const Int64 nowTick = ::GetTickCount64();
                // 실행 시간에 도달하지 않은 경우
                if (nowTick < item.execTick)
                {
                    waitMs = item.execTick - nowTick;
                    break;
                }

                mExecItems.emplace_back(item);
                mScheduledItems.pop();
            }
        }

        const Int64 startTick = ::GetTickCount64();

        // 꺼낸 모든 아이템의 job을 큐에 넣음
        for (Item& item : mExecItems)
        {
            item.queue.lock()->Push(std::move(item.job));
        }
        mExecItems.clear();

        const Int64 endTick = ::GetTickCount64();
        // 대기 시간 갱신
        waitMs -= endTick - startTick;
        if (waitMs < 0)
        {
            waitMs = 0;
        }

        return waitMs;
    }

    /**
     * 타이머 쓰레드의 메인 루프입니다.
     *
     * 이 함수는 별도의 스레드에서 호출되어 계속 실행되면서 예약된 작업들을 분배합니다.
     *
     * 동작:
     * 1. 이미 실행 중인지 확인하고, 실행 상태(mRunning)로 설정합니다.
     * 2. 지속적으로 Distribute()를 호출하여 실행 시간이 된 작업들을 분배합니다.
     * 3. SRWLOCK을 사용한 배타적 락을 획득하여 신호 상태를 확인합니다.
     * 4. 신호가 없으면(mWaked가 false) SleepConditionVariableSRW로 대기합니다.
     * 5. 신호 도착 또는 타임아웃으로 깨어나면 mWaked를 false로 리셋합니다.
     * 6. mRunning이 false가 될 때까지 이 과정을 반복합니다.
     */
    void JobTimer::Run()
    {
        ASSERT_CRASH(mRunning == false, "ALREADY_RUNNING");
        mRunning = true;

        while (mRunning)
        {
            // 타이머 설정 시간이 지난 잡을 큐에 분배
            Int64 waitMs = Distribute();

            // 배타적 락 획득
            SrwLockWriteGuard guard(mLock);

            // 이미 신호를 처리했거나 신호가 아직 없는 경우
            if (!mWaked)
            {
                // 신호 도착 또는 타임아웃까지 대기
                BOOL result = ::SleepConditionVariableSRW(
                    &mCondVar,                      // 조건 변수 구조체 주소
                    &mLock,                         // 단일 SRWLOCK 사용
                    static_cast<DWORD>(waitMs),     // 대기 시간(밀리초)
                    0                               // 플래그(0은 독점 모드)
                );
            }
            // 신호를 처리했으므로 플래그 리셋
            mWaked = false;
        }
    }
} // namespace core
