/*    Core/Job/Timer.h    */

#pragma once

namespace core
{
    /*
     * JobTimer 클래스는 예약된 시간에 작업을 실행하기 위한 스케줄링 시스템입니다.
     *
     * 주요 특징:
     * - 지정된 지연 시간 후에 JobQueue에 Job을 푸시하도록 스케줄링합니다.
     * - 우선순위 큐를 사용하여 실행 시간이 빠른 작업부터 처리합니다.
     * - Schedule() 메서드로 작업을 예약할 수 있습니다.
     * - Distribute()는 실행 시간에 도달한 작업들을 해당 JobQueue로 분배합니다.
     * - Run() 메서드로 타이머 스레드를 시작하여 지속적으로 작업을 분배합니다.
     * - SRWLOCK을 사용한 통합된 동기화 방식 제공
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
        SRWLOCK                 mLock;
        CONDITION_VARIABLE      mCondVar;
        Bool                    mWaked = false;

        PriorityQueue<Item>     mScheduledItems;
        Bool                    mRunning = false;
        Vector<Item>            mExecItems;

        static constexpr Int64  kMaxWaitMs = 100;
    };
} // namespace core
