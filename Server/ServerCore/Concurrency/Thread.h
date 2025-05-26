/*    ServerCore/Concurrency/Thread.h    */

#pragma once

namespace core
{
    /**
     * ThreadManager - 서버의 스레드 생성 및 관리 클래스
     *
     * 주요 기능:
     * - 콜백 함수를 사용한 비동기 스레드 생성 및 실행
     * - 생성된 모든 스레드의 수명 주기 관리
     * - 스레드 로컬 스토리지(TLS) 초기화
     * - 스레드 안전한 작업을 위한 내부 동기화
     *
     * 사용 패턴:
     * - Launch() 메서드로 작업 스레드 생성
     * - 소멸자에서 자동으로 모든 스레드 조인
     */
    class ThreadManager
    {
    public:
        ThreadManager();
        ~ThreadManager();

        void                Launch(Function<void(void)> callback);
        void                Join();

        static void         InitTls();
        static void         DestroyTls();

    private:
        SRWLOCK             mLock;
        Vector<Thread>      mThreads;
    };
} // namespace core
