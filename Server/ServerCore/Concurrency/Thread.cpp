/*    ServerCore/Concurrency/Thread.cpp    */

#include "ServerCore/Pch.h"
#include "ServerCore/Concurrency/Thread.h"

namespace core
{
    /**
     * ThreadManager 생성자
     *
     * 메인 스레드의 TLS를 초기화합니다.
     * 이를 통해 메인 스레드에 고유 ID를 할당합니다.
     */
    ThreadManager::ThreadManager()
    {
        InitTls();
    }

    /**
     * ThreadManager 소멸자
     *
     * 관리 중인 모든 스레드가 완료될 때까지 대기합니다.
     * 소멸 시 생성한 스레드와 TLS 정리를 보장합니다.
     */
    ThreadManager::~ThreadManager()
    {
        Join();
        DestroyTls();
    }

    /**
     * 새 스레드 생성 및 실행
     *
     * 제공된 콜백 함수를 실행하는 새 스레드를 생성합니다.
     * 스레드 실행 전후로 TLS 초기화 및 정리 작업을 자동으로 수행합니다.
     *
     * @param callback 스레드에서 실행할 작업 함수
     */
    void ThreadManager::Launch(Function<void(void)> callback)
    {
        SrwLockWriteGuard guard(mLock);

        mThreads.emplace_back([callback]()
                              {
                                  InitTls();
                                  callback();
                                  DestroyTls();
                              });
    }

    /**
     * 모든 스레드 조인
     *
     * 관리 중인 모든 스레드가 완료될 때까지 대기합니다.
     * 모든 스레드가 조인된 후 스레드 목록을 정리합니다.
     */
    void ThreadManager::Join()
    {
        SrwLockWriteGuard guard(mLock);

        for (Thread& thread : mThreads)
        {
            if (thread.joinable())
                thread.join();
        }

        mThreads.clear();
    }

    /**
     * 스레드 로컬 스토리지(TLS) 초기화
     *
     * 현재 스레드에 유일한 ID를 할당합니다.
     * 모든 스레드는 시작 시 반드시 이 함수를 호출해야 합니다.
     */
    void ThreadManager::InitTls()
    {
        // 스레드 id 발급
        static Atomic<Int32> sThreadId = 1;
        tThreadId = sThreadId.fetch_add(1);
        ASSERT_CRASH(tThreadId > 0, "THREAD_ID_OVERFLOW");
    }

    /**
     * 스레드 로컬 스토리지(TLS) 정리
     *
     * 스레드 ID를 초기화하여 TLS 리소스를 정리합니다.
     * 모든 스레드는 종료 전 반드시 이 함수를 호출해야 합니다.
     */
    void ThreadManager::DestroyTls()
    {
        // 스레드 id 초기화
        tThreadId = 0;
    }
} // namespace core
