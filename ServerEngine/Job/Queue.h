/*    ServerEngine/Job/Queue.h    */

#pragma once

#include <concurrentqueue/concurrentqueue.h>

/*
 * 비동기 작업을 Job 형태로 만들 수 있다.
 */
class Job
{
public:
    using CallbackType  = Function<void()>;

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
 * Job을 직렬화하기 위해 사용하는 큐
 * 여러 스레드가 Job을 Push할 수 있지만, Flush하는 스레드는 유일해야 한다.
 */
class JobQueue
    : public std::enable_shared_from_this<JobQueue>
{
public:
    JobQueue();

    void        Push(SharedPtr<Job> job);
    Bool        TryFlush(Int64 timeoutMs);

private:
    moodycamel::ConcurrentQueue<SharedPtr<Job>>    mQueue;
    Atomic<Int64>                                  mJobCount;

    static constexpr Int64      kInitQueueSize = 128;
};

/*
 * JobQueueManager는 IOCP(I/O Completion Port)를 사용하여 비동기 작업 큐를 관리하는 클래스
 *
 * 주요 기능:
 * - 작업 큐(JobQueue)를 관리하고 큐에 담긴 작업(Job)을 비동기적으로 실행
 * - IOCP를 활용해 효율적인 멀티스레딩 작업 처리 구조 제공
 * - 작업 큐가 비어있다가 새 작업이 추가될 때 자동 등록 기능
 *
 * 동작 방식:
 * 1. JobQueue::Push()를 통해 큐에 첫 작업이 추가되면 JobQueueManager::Register()가 호출됨
 * 2. Register()는 큐를 RegisterEvent로 래핑하여 IOCP에 등록
 * 3. FlushQueues() 메서드는 IOCP에서 완료된 이벤트를 가져와 해당 큐의 작업을 처리
 * 4. 큐의 모든 작업이 처리되면 이벤트 객체를 해제하고, 처리하지 못했으면 다시 IOCP에 등록
 *
 * 이 클래스는 여러 스레드가 작업을 등록하고, 별도의 스레드(들)에서 작업을 실행하는
 * 생산자-소비자 패턴을 IOCP 기반으로 구현한 것입니다.
 */
class JobQueueManager
{
public:
    struct RegisterEvent
        : public OVERLAPPED
    {
        SharedPtr<JobQueue> queue;

        void Init()
        {
            Internal = 0;
            InternalHigh = 0;
            Offset = 0;
            OffsetHigh = 0;
            hEvent = NULL;
        }
    };

public:
    JobQueueManager();
    ~JobQueueManager();

    void            Register(SharedPtr<JobQueue> queue);
    void            FlushQueues(UInt32 timeoutMs = INFINITE);
    void            HandleError(Int64 errorCode);

private:
    void            PostRegisterEvent(RegisterEvent* event);

private:
    HANDLE          mIocp = nullptr;
    Bool            mRunning = false;
    Atomic<Int64>   mEventCount = 0;

    static constexpr Int64      kFlushTimeoutMs = 100;
};
