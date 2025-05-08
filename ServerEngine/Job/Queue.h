/*    ServerEngine/Job/Queue.h    */

#pragma once

#include "ServerEngine/Concurrency/Queue.h"

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
 * JobSerializer가 Job을 직렬화하기 위해 사용하는 큐
 * 여러 스레드가 Job을 Push할 때, 최초 Job을 Push한 스레드가 모든 Job을 처리한다.
 */
class JobQueue
    : public std::enable_shared_from_this<JobQueue>
{
public:
    void        Push(SharedPtr<Job> job, Bool canFlush = true);

public:
    // 큐에 남은 job이 없을 때까지 처리
    void        Flush();

private:
    LockQueue<SharedPtr<Job>>       mJobs;
    Atomic<Int64>                   mJobCount = 0;
};

/*
 * Job을 처리하기 벅찬 스레드는 JobQueue를 ReservedJobManager에 예약한다.
 * ProcessJobs()를 호출하는 스레드가 예약된 JobQueue를 처리한다.
 */
class ReservedJobManager
{
public:
    void        ReserveJobs(SharedPtr<JobQueue> jobs);
    void        ProcessJobs();

private:
    LockQueue<SharedPtr<JobQueue>>  mJobs;
};
