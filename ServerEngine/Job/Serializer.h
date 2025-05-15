/*    ServerEngine/Job/Serializer.h    */

#pragma once

#include "ServerEngine/Job/Queue.h"
#include "ServerEngine/Job/Timer.h"

/*
 * JobSerializer를 상속받은 클래스의 인스턴스는 자신만의 JobQueue를 소유한다.
 * 바로 JobQueue에 Push할 수 있으며, 일정 시간 후에 JobQueue에 Push할 수도 있다.
 */
class JobSerializer
    : public std::enable_shared_from_this<JobSerializer>
{
public:
    void PushJob(Job::CallbackType&& callback)
    {
        mQueue->Push(std::make_shared<Job>(std::move(callback)));
    }

    template<typename T, typename Ret, typename... Args>
    void PushJob(Ret(T::* method)(Args...), Args... args)
    {
        SharedPtr<T> owner = std::static_pointer_cast<T>(shared_from_this());
        mQueue->Push(std::make_shared<Job>(std::move(owner), method, std::forward<Args>(args)...));
    }

    void ScheduleJob(Int64 delayMs, Job::CallbackType&& callback)
    {
        SharedPtr<Job> job = std::make_shared<Job>(std::move(callback));
        gJobTimer->Schedule(std::move(job), mQueue, delayMs);
    }

    template<typename T, typename Ret, typename... Args>
    void ScheduleJob(Int64 delayMs, Ret(T::* method)(Args...), Args... args)
    {
        SharedPtr<T> owner = std::static_pointer_cast<T>(shared_from_this());
        SharedPtr<Job> job = std::make_shared<Job>(std::move(owner), method, std::forward<Args>(args)...);
        gJobTimer->Schedule(std::move(job), mQueue, delayMs);
    }

protected:
    SharedPtr<JobQueue>     mQueue = std::make_shared<JobQueue>();
};
