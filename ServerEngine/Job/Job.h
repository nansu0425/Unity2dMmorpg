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
        auto tuple = std::make_tuple(std::forward<Args>(args)...);

        mCallback = [obj = std::move(obj), method, tuple = std::move(tuple)]() mutable
            {
                // tuple의 요소를 unpack하여 인자로 전달
                std::apply([obj, method](auto&&... args)
                           {    
                               return (obj.get()->*method)(std::forward<decltype(args)>(args)...);
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
