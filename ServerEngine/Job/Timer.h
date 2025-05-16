/*    ServerEngine/Job/Timer.h    */

#pragma once

/*
 * JobTimer 클래스는 예약된 시간에 작업을 실행하기 위한 스케줄링 시스템입니다.
 *
 * 주요 특징:
 * - 지정된 지연 시간 후에 JobQueue에 Job을 푸시하도록 스케줄링합니다.
 * - 우선순위 큐를 사용하여 실행 시간이 빠른 작업부터 처리합니다.
 * - Schedule() 메서드로 작업을 예약할 수 있습니다.
 * - Distribute()는 실행 시간이 도래한 작업들을 해당 JobQueue로 분배합니다.
 * - Run() 메서드로 타이머 스레드를 시작하여 지속적으로 작업을 분배합니다.
 * - 멀티스레딩 환경에서 안전하게 동작하기 위한 읽기/쓰기 락을 사용합니다.
 * - 조건 변수를 활용한 효율적인 스레드 동기화를 구현합니다.
 */
class JobTimer
{
public:
    struct Item
    {
        Int64               execTick;
        SharedPtr<Job>      job;
        WeakPtr<JobQueue>   queue;

        bool    operator<(const Item& other) const { return (execTick > other.execTick); }
    };

public:
    JobTimer();
    ~JobTimer();

    void        Schedule(SharedPtr<Job> job, WeakPtr<JobQueue> queue, Int64 delayMs);
    Int64       Distribute();
    void        Run();

private:
    RW_LOCK;
    PriorityQueue<Item>     mScheduledItems;
    Bool                    mRunning = false;
    Vector<Item>            mExecItems;

    // 조건 변수 기반 동기화
    std::mutex              mWakeMutex;
    std::condition_variable mWakeCV;
    bool                    mNotified = false;

    static constexpr Int64  kMaxWaitMs = 100;
};
