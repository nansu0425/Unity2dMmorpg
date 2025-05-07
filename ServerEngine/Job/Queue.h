/*    ServerEngine/Job/Queue.h    */

#pragma once

#include "ServerEngine/Concurrency/Queue.h"

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
 * LockQueue를 사용하여 Job을 저장하는 큐
 */
class JobQueue
{
public:
    void Push(SharedPtr<Job> job)
    {
        mJobs.Push(std::move(job));
    }

    SharedPtr<Job> Pop()
    {
        SharedPtr<Job> job = nullptr;
        mJobs.Pop(job);

        return job;
    }

private:
    LockQueue<SharedPtr<Job>>  mJobs;
};
