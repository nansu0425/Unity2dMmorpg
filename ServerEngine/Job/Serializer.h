/*    ServerEngine/Job/Serializer.h    */

#pragma once

#include "ServerEngine/Job/Job.h"

class JobSerializer
    : public std::enable_shared_from_this<JobSerializer>
{
public:
    void PushJob(Job::CallbackType&& callback)
    {
        SharedPtr<Job> job = std::make_shared<Job>(std::move(callback));
        mJobs.Push(std::move(job));
    }

    // JobSerializer를 상속받은 객체의 메서드 호출을 Job 큐에 넣는다
    template<typename T, typename Ret, typename... Args>
    void PushJob(Ret(T::* method)(Args...), Args... args)
    {
        SharedPtr<T> obj = std::static_pointer_cast<T>(shared_from_this());
        SharedPtr<Job> job = std::make_shared<Job>(std::move(obj), method, std::forward<Args>(args)...);
        mJobs.Push(std::move(job));
    }

public:
    virtual void FlushJobs() = 0;

protected:
    JobQueue    mJobs;
};
