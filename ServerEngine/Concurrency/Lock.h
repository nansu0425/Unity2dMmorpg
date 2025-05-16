/*    ServerEngine/Concurrency/Lock.h    */

#pragma once

/*
 * RwSpinLock - 읽기-쓰기 스핀 락 구현
 *
 * 락 비트 플래그 구조: [WRRR'RRRR]
 * W: 쓰기 잠금 플래그 (0xF000'0000)
 * R: 읽기 잠금 카운트 (0x0FFF'FFFF)
 *
 * 특징:
 * - 가벼운 읽기 작업이 많은 경우 최적화된 성능
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

private:
    SRWLOCK&    mLock;
};

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

private:
    SRWLOCK&    mLock;
};
