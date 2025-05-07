/*    ServerEngine/Job/Serializer.h    */

#pragma once

#include "ServerEngine/Job/Queue.h"

/*
 * JobSerializer를 상속받은 클래스는 메서드 호출을 Job 큐에 넣을 수 있다.
 * Job 큐에 있는 Job 처리는 FlushJobs()에서 구현한다.
 * PushJob()은 thread-safe하지만, FlushJobs()는 싱글 스레드에서만 호출되어야 한다.
 */
class JobSerializer
    : public std::enable_shared_from_this<JobSerializer>
{
public:
    void PushJob(Job::CallbackType&& callback)
    {
        SharedPtr<Job> job = std::make_shared<Job>(std::move(callback));
        mJobs.Push(std::move(job));
    }

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
