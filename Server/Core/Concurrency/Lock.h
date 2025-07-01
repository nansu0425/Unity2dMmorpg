/*    Core/Concurrency/Lock.h    */

#pragma once

namespace core
{
    /*
     * RwSpinLock - 읽기-쓰기 스핀 락 구현
     *
     * 락 비트 플래그 구조: [WRRR'RRRR]
     * W: 쓰기 잠금 플래그 (0xF000'0000)
     * R: 읽기 잠금 카운트 (0x0FFF'FFFF)
     *
     * 특징:
     * - 쓰기 작업 시 배타적 잠금, 읽기 작업 시 공유 잠금
     * - 스핀 대기 방식으로 짧은 임계 구역에 적합
     * - 재귀 잠금 금지(정의되지 않은 동작 발생)
     * - 디버그 모드에서 데드락 감지 지원
     */
    class RwSpinLock
    {
    public:
        void    LockWrite(const Char8* name);
        void    UnlockWrite(const Char8* name);
        void    LockRead(const Char8* name);
        void    UnlockRead(const Char8* name);

    public:
        class WriteGuard
        {
        public:
            WriteGuard(RwSpinLock& lock, const Char8* name)
                : mLock(lock)
                , mName(name)
            {
                mLock.LockWrite(mName);
            }

            ~WriteGuard()
            {
                mLock.UnlockWrite(mName);
            }

            // 복사 금지
            WriteGuard(const WriteGuard&) = delete;
            WriteGuard& operator=(const WriteGuard&) = delete;

        private:
            RwSpinLock& mLock;
            const Char8* mName;
        };

        class ReadGuard
        {
        public:
            ReadGuard(RwSpinLock& lock, const Char8* name)
                : mLock(lock)
                , mName(name)
            {
                mLock.LockRead(mName);
            }

            ~ReadGuard()
            {
                mLock.UnlockRead(mName);
            }

            // 복사 금지
            ReadGuard(const ReadGuard&) = delete;
            ReadGuard& operator=(const ReadGuard&) = delete;

        private:
            RwSpinLock& mLock;
            const Char8* mName;
        };

    private:
        static constexpr UInt64     kWriteFlag = 0xF000'0000;
        static constexpr UInt64     kEmptyFlag = 0x0000'0000;
        static constexpr UInt64     kReadCountMask = 0x0FFF'FFFF;
        static constexpr Int32      kLockTimeoutMs = 10'000;
        static constexpr Int32      kMaxBackoff = 1'024;

    private:
        Atomic<UInt64>              mLockFlag = kEmptyFlag;
    };

    /*
     * SrwLockWriteGuard - Windows SRWLOCK의 독점 잠금(쓰기 잠금)을 위한 RAII 래퍼
     *
     * 특징:
     * - RAII 패턴을 통해 스코프 기반 자동 락 관리
     * - 생성자에서 독점 락 획득, 소멸자에서 자동 해제
     * - Windows의 기본 SRW 락 API 사용 (AcquireSRWLockExclusive/ReleaseSRWLockExclusive)
     * - 복사 생성자와 대입 연산자 명시적으로 삭제 (락의 소유권 복제 방지)
     *
     * 사용 예시:
     * {
     *     SrwLockWriteGuard guard(srwLock);
     *     // 보호된 코드 실행
     * } // 스코프 종료 시 자동으로 락 해제
     */
    class SrwLockWriteGuard
    {
    public:
        SrwLockWriteGuard(SRWLOCK& lock)
            : mLock(lock)
        {
            ::AcquireSRWLockExclusive(&mLock);
        }

        ~SrwLockWriteGuard()
        {
            ::ReleaseSRWLockExclusive(&mLock);
        }

        // 복사 금지
        SrwLockWriteGuard(const SrwLockWriteGuard&) = delete;
        SrwLockWriteGuard& operator=(const SrwLockWriteGuard&) = delete;

    private:
        SRWLOCK& mLock;
    };

    /*
     * SrwLockReadGuard - Windows SRWLOCK의 공유 잠금(읽기 잠금)을 위한 RAII 래퍼
     *
     * 특징:
     * - RAII 패턴을 통해 스코프 기반 자동 락 관리
     * - 생성자에서 공유 락 획득, 소멶자에서 자동 해제
     * - Windows의 기본 SRW 락 API 사용 (AcquireSRWLockShared/ReleaseSRWLockShared)
     * - 복사 생성자와 대입 연산자 명시적으로 삭제 (락의 소유권 복제 방지)
     * - 여러 스레드가 동시에 읽기 락을 획득할 수 있음
     *
     * 사용 예시:
     * {
     *     SrwLockReadGuard guard(srwLock);
     *     // 읽기 전용 작업 수행
     * } // 스코프 종료 시 자동으로 락 해제
     */
    class SrwLockReadGuard
    {
    public:
        SrwLockReadGuard(SRWLOCK& lock)
            : mLock(lock)
        {
            ::AcquireSRWLockShared(&mLock);
        }

        ~SrwLockReadGuard()
        {
            ::ReleaseSRWLockShared(&mLock);
        }

        // 복사 금지
        SrwLockReadGuard(const SrwLockReadGuard&) = delete;
        SrwLockReadGuard& operator=(const SrwLockReadGuard&) = delete;

    private:
        SRWLOCK& mLock;
    };
} // namespace core
