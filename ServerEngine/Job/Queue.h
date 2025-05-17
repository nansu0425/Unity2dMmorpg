/*    ServerEngine/Job/Queue.h    */

#pragma once

#include <concurrentqueue/concurrentqueue.h>

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

class JobQueue;

/*
 * JobPushEvent는 IOCP(I/O Completion Port)와 함께 사용되는 이벤트 구조체입니다.
 * OVERLAPPED 구조체를 상속하여 Windows IOCP 시스템에 등록 가능합니다.
 *
 * 주요 특징:
 * - JobQueue에 대한 shared_ptr 참조를 안전하게 소유
 * - IOCP 이벤트를 위한 OVERLAPPED 멤버 변수들 초기화
 * - 비동기 작업 완료 후 원본 JobQueue로 라우팅하는 메커니즘 제공
 * - 각 JobQueue는 자체 JobPushEvent 인스턴스를 가짐
 */
struct JobPushEvent
    : public OVERLAPPED
{
    SharedPtr<JobQueue> owner;

    void Init(SharedPtr<JobQueue> queue)
    {
        Internal = 0;
        InternalHigh = 0;
        Offset = 0;
        OffsetHigh = 0;
        hEvent = NULL;
        owner = std::move(queue);
    }
};

/*
 * JobQueue는 Job을 직렬화하여 순차적으로 실행하기 위한 큐입니다.
 * 락프리 큐(moodycamel::ConcurrentQueue)를 사용하여 멀티스레드 환경에서 안전하게 동작합니다.
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
    JobPushEvent*               GetPushEvent() { return &mPushEvent; }

private:
    moodycamel::ConcurrentQueue<SharedPtr<Job>>    mQueue;
    Atomic<Int64>                                  mJobCount;
    JobPushEvent                                   mPushEvent;

    static constexpr Int64      kInitQueueSize = 128;
};

/*
 * JobQueueManager는 IOCP(I/O Completion Port)를 사용하여 여러 JobQueue의 작업 처리를 관리합니다.
 * 효율적인 스레드 관리와 작업 분배를 통해 고성능 비동기 작업 처리를 제공합니다.
 *
 * 주요 기능:
 * - IOCP 기반의 최적화된 비동기 이벤트 처리 메커니즘
 * - 여러 큐의 동시 등록 지원 및 원자적 큐 카운트 관리
 * - 미완료 작업이 있는 큐의 자동 재등록으로 모든 작업 완료 보장
 * - 오류 상황 자동 감지 및 로깅을 통한 안정성 향상
 * - 설정 가능한 타임아웃으로 리소스 사용 제어 및 최적화
 */
class JobQueueManager
{
public:
    JobQueueManager();
    ~JobQueueManager();

    void                        RegisterQueue(SharedPtr<JobQueue> queue);
    void                        FlushQueues(UInt32 timeoutMs = INFINITE);

private:
    void                        PostPushEvent(SharedPtr<JobQueue> queue);
    void                        HandleError(Int64 errorCode);

private:
    HANDLE                      mIocp = nullptr;
    Bool                        mRunning = false;
    Atomic<Int64>               mQueueCount = 0;

    static constexpr Int64      kFlushTimeoutMs = 100;
};
