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

class JobQueue
{
public:
    void Push(SharedPtr<Job> job)
    {
        const Int64 prevCount = mJobCount.fetch_add(1);
        mJobs.Push(std::move(job));

        // 처음 job을 넣은 스레드가 flush를 담당
        if (prevCount == 0)
        {
            FlushJobs();
        }
    }

private:
    // 큐에 남은 job이 없을 때까지 처리
    void FlushJobs()
    {
        Vector<SharedPtr<Job>> jobs;
        while (true)
        {
            mJobs.PopAll(jobs);
            Int64 jobCount = 0;
            // 모든 job을 실행
            for (auto& job : jobs)
            {
                job->Execute();
                ++jobCount;
            }
            // job이 없으면 종료
            if (mJobCount.fetch_sub(jobCount) == jobCount)
            {
                break;
            }
            jobs.clear();
        }
    }

private:
    LockQueue<SharedPtr<Job>>   mJobs;
    Atomic<Int64>               mJobCount = 0;
};
