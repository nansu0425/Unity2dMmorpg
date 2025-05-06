/*    ServerEngine/Job/Job.h    */

#pragma once

class Job
{
public:
    using CallbackType  = Function<void()>;

public:
    Job(CallbackType&& callback)
        : mCallback(std::move(callback))
    {}

    template<typename T, typename Ret, typename... Args>
    Job(SharedPtr<T> obj, Ret(T::* method)(Args...), Args&&... args)
    {
        mCallback = [obj = std::move(obj), method, args...]()
            {
                (obj.get()->*method)(args...);
            };
    }

    void Execute()
    {
        mCallback();
    }

private:
    CallbackType    mCallback;
};
