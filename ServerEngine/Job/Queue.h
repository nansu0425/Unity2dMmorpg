/*    ServerEngine/Job/Queue.h    */

#pragma once

class Job
{
public:
    using CallbackType = Function<void()>;

public:
    Job(CallbackType&& callback)
        : mCallback(std::move(callback))
    {}

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
        WRITE_GUARD;
        mJobs.push(std::move(job));
    }

    SharedPtr<Job> Pop()
    {
        SharedPtr<Job> job = nullptr;

        WRITE_GUARD;
        if (false == mJobs.empty())
        {
            job = mJobs.front();
            mJobs.pop();
        }

        return job;
    }

private:
    RW_LOCK;
    Queue<SharedPtr<Job>>  mJobs;
};
