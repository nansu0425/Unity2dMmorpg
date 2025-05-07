/*    ServerEngine/Job/Serializer.h    */

#pragma once

#include "ServerEngine/Job/Queue.h"

/*
 * JobSerializer를 상속받은 클래스는 비동기 작업을 Job 형태로 만들 수 있다.
 * 만들어진 Job들은 JobQueue를 이용해 직렬화된다.
 * 여러 스레드가 Job을 Push할 때, 최초 Job을 Push한 스레드가 모든 Job을 처리한다.
 */
class JobSerializer
    : public std::enable_shared_from_this<JobSerializer>
{
public:
    void MakeJob(Job::CallbackType&& callback)
    {
        mJobs.Push(std::make_shared<Job>(std::move(callback)));
    }

    template<typename T, typename Ret, typename... Args>
    void MakeJob(Ret(T::* method)(Args...), Args... args)
    {
        SharedPtr<T> obj = std::static_pointer_cast<T>(shared_from_this());
        mJobs.Push(std::make_shared<Job>(std::move(obj), method, std::forward<Args>(args)...));
    }

protected:
    JobQueue        mJobs;
};
