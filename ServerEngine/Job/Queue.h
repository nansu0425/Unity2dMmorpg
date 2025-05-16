/*    ServerEngine/Job/Queue.h    */

#pragma once

#include <concurrentqueue/concurrentqueue.h>

struct JobPushEvent;

/*
 * Job 클래스는 비동기 작업을 실행 가능한 객체로 캡슐화합니다.
 * 일반 함수, 람다 또는 클래스 메서드를 저장하고 나중에 실행할 수 있습니다.
 *
 * 주요 특징:
 * - 일반 함수 또는 람다 표현식을 저장하는 생성자
 * - 특정 객체의 메서드를 인자와 함께 호출할 수 있는 생성자 (템플릿 기반)
 * - Execute() 메서드로 저장된 작업을 실행
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
 * 락프리 큐(moodycamel::ConcurrentQueue)를 사용하여 멀티스레드 환경에서 안전하게 동작합니다.
 *
 * 주요 특징:
 * - 여러 스레드에서 동시에 Job을 Push 가능
 * - TryFlush 메서드로 큐에 있는 작업을 지정된 시간 내에 실행
 * - 첫 작업이 추가될 때 자동으로 JobQueueManager에 등록되어 작업 처리를 보장
 * - 모든 이벤트는 전역 JobPushEventPool에서 관리
 */
class JobQueue
    : public std::enable_shared_from_this<JobQueue>
{
public:
    JobQueue();

    void            Push(SharedPtr<Job> job);
    Bool            TryFlush(Int64 timeoutMs);

private:
    moodycamel::ConcurrentQueue<SharedPtr<Job>>    mQueue;
    Atomic<Int64>                                  mJobCount;

    static constexpr Int64      kInitQueueSize = 128;
};

/*
 * JobQueueManager는 IOCP(I/O Completion Port)를 사용하여 여러 JobQueue의 작업 처리를 관리합니다.
 * 효율적인 스레드 관리와 작업 분배를 통해 고성능 비동기 작업 처리를 제공합니다.
 *
 * 주요 기능:
 * - IOCP 기반의 최적화된 스레드 관리 (커널 수준 스케줄링)
 * - 여러 스레드가 동시에 작업을 등록하고, 별도의 처리 스레드에서 실행
 * - 자동화된 큐 등록 및 작업 처리 메커니즘
 * - 효율적인 문맥 전환과 스레드 재사용을 통한 성능 최적화
 */
class JobQueueManager
{
public:
    JobQueueManager();
    ~JobQueueManager();

    void            RegisterQueue(SharedPtr<JobQueue> queue);
    void            FlushQueues(UInt32 timeoutMs = INFINITE);

private:
    void            PostPushEvent(JobPushEvent* event);
    void            HandleError(Int64 errorCode);

private:
    HANDLE                      mIocp = nullptr;
    Bool                        mRunning = false;
    Atomic<Int64>               mQueueCount = 0;

    static constexpr Int64      kFlushTimeoutMs = 100;
};
