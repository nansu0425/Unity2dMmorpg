/*    ServerCore/Job/Queue.h    */

#pragma once

namespace core
{
    /*
     * Job 클래스는 비동기 작업을 실행 가능한 객체로 캡슐화합니다.
     * 일반 함수, 람다 또는 클래스 메서드를 저장하고 나중에 실행할 수 있습니다.
     *
     * 주요 특징:
     * - 일반 함수 또는 람다 표현식을 저장하는 생성자 제공
     * - 특정 객체의 메서드를 인자와 함께 호출할 수 있는 템플릿 기반 생성자 지원
     * - std::tuple과 std::apply를 활용한 임의 개수의 인자 완벽 전달
     * - Execute() 메서드를 통한 저장된 작업의 실행
     */
    class Job
    {
    public:
        using CallbackType = Function<void()>;

    public:
        // 함수를 호출하는 Job을 생성
        Job(CallbackType&& callback)
            : mCallback(std::move(callback))
        {}

        // 특정 객체의 메서드를 호출하는 Job을 생성
        template<typename T, typename Ret, typename... Args>
        Job(SharedPtr<T> owner, Ret(T::* method)(Args...), Args&&... args)
        {
            auto tuple = std::make_tuple(std::forward<Args>(args)...);

            mCallback = [owner = std::move(owner), method, tuple = std::move(tuple)]() mutable
                {
                    // tuple의 요소를 unpack하여 인자로 전달
                    std::apply([owner = std::move(owner), method](auto&&... args)
                               {
                                   (owner.get()->*method)(std::forward<decltype(args)>(args)...);
                               },
                               std::move(tuple));
                };
        }

        void Execute()
        {
            mCallback();
        }

    private:
        CallbackType    mCallback;
    };

    /*
     * JobQueue는 Job을 직렬화하여 순차적으로 실행하기 위한 큐입니다.
     * 락프리 큐를 사용하여 멀티스레드 환경에서 안전하게 동작합니다.
     *
     * 주요 특징:
     * - 원자적 카운터를 통한 작업 수 추적으로 경합 상태 방지
     * - 여러 스레드에서 동시에 Job을 Push 가능
     * - 큐가 빈 상태에서 첫 작업 추가 시 자동으로 JobQueueManager에 등록
     * - TryFlush 메서드로 큐의 작업을 지정된 시간 내에 실행하고 완료 여부 반환
     * - std::enable_shared_from_this를 통한 안전한 self-reference 제공
     */
    class JobQueue
        : public std::enable_shared_from_this<JobQueue>
    {
    public:
        JobQueue();

        void                        Push(SharedPtr<Job> job);
        Bool                        TryFlush(Int64 timeoutMs);

    private:
        LockfreeQueue<SharedPtr<Job>>       mQueue;
        Atomic<Int64>                       mJobCount;

        static constexpr Int64      kInitQueueSize = 64;
    };

    /*
     * JobQueueManager는 작업 큐를 관리하고 작업을 효율적으로 처리합니다.
     * 락프리 큐와 Windows 네이티브 동기화 객체를 사용하여 고성능 비동기 작업 처리를 제공합니다.
     *
     * 주요 기능:
     * - 락프리 큐를 통한 효율적인 작업 큐 관리
     * - SRWLOCK과 CONDITION_VARIABLE을 활용한 최적화된 동기화
     * - 여러 스레드 간의 작업 분배
     * - 미완료 작업이 있는 큐의 자동 재등록으로 모든 작업 완료 보장
     * - 대기 중인 스레드의 효율적인 깨우기를 통한 성능 최적화
     */
    class JobQueueManager
    {
    public:
        JobQueueManager();

        void                        RegisterQueue(SharedPtr<JobQueue> queue);
        void                        FlushQueues();

    private:
        SRWLOCK                                 mLock;
        CONDITION_VARIABLE                      mCondVar;
        Bool                                    mWaked = false;
        LockfreeQueue<SharedPtr<JobQueue>>      mQueues;
        Bool                                    mRunning = true;

        static constexpr Int64      kFlushTimeoutMs = 100;
        static constexpr Int64      kInitQueueSize = 128;
    };
} // namespace core
