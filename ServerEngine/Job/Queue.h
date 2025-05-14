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
    Job(SharedPtr<T> obj, Ret(T::* method)(Args...), Args&&... args)
    {
        auto tuple = std::make_tuple(std::forward<Args>(args)...);

        mCallback = [obj = std::move(obj), method, tuple = std::move(tuple)]() mutable
            {
                // tuple의 요소를 unpack하여 인자로 전달
                std::apply([obj, method](auto&&... args)
                           {
                               (obj.get()->*method)(std::forward<decltype(args)>(args)...);
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

    // void        Push(SharedPtr<Job> job, Bool canFlush = true);
    void        Push(SharedPtr<Job> job);
    Bool        Flush(UInt64 timeoutMs);

private:
    enum Constants
    {
        kQueueSize  = 128,
    };

private:
    moodycamel::ConcurrentQueue<SharedPtr<Job>>    mQueue;
    Atomic<Int64>                                  mJobCount;
};

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

    void        Register(SharedPtr<JobQueue> queue);
    void        FlushQueues(UInt32 timeoutMs = INFINITE);
    void        HandleError(Int64 errorCode);

private:
    void        PostRegisterEvent(RegisterEvent* event);

private:
    HANDLE      mIocp = nullptr;
    Bool        mRunning = false;

    static constexpr Int64      kFlushTimeoutMs = 10;
};

///*
// * JobQueue를 Flush하기 힘든 스레드는 JobQueue를 JobQueueManager에 등록한다.
// */
//class JobQueueManager
//{
//public:
//    void        Register(SharedPtr<JobQueue> queue);
//    void        FlushQueues();
//
//private:
//    LockQueue<SharedPtr<JobQueue>>  mQueues;
//};
