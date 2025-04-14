/*    ServerEngine/Concurrency/Lock.h    */

#pragma once

/*
 * 락 비트 플래그 구조: [WRRRRRRR]
 * W: 쓰기 잠금 여부
 * RRRRRRR: 읽기 잠금 개수
 * 재귀 잠금 금지(정의되지 않은 동작 발생)
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

    private:
        RwSpinLock&     mLock;
        const Char8*    mName;
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

    private:
        RwSpinLock&     mLock;
        const Char8*    mName;
    };

private:
    enum Flag : Byte
    {
        kWriteState         = 0b1000'0000,
        kReadCountMask      = 0b0111'1111,
        kEmpty              = 0b0000'0000,
    };

    static constexpr Int32  kLockTimeoutTick = 10'000;
    static constexpr Int32  kMaxBackoff = 1'024;

private:
    Atomic<Byte>    mLockFlag = Flag::kEmpty;
};

/*
 * 윈도우의 Slim Reader/Writer lock을 래핑
 * 재귀 잠금 불가능
 */
class SrwLock
{
public:
    void    AcquireWriteLock(const Char8* name);
    void    ReleaseWriteLock(const Char8* name);
    void    AcquireReadLock(const Char8* name);
    void    ReleaseReadLock(const Char8* name);

public:
    class WriteGuard
    {
    public:
        WriteGuard(SrwLock& lock, const Char8* name)
            : mLock(lock)
            , mName(name)
        {
            mLock.AcquireWriteLock(mName);
        }

        ~WriteGuard()
        {
            mLock.ReleaseWriteLock(mName);
        }

    private:
        SrwLock&        mLock;
        const Char8*    mName;
    };

    class ReadGuard
    {
    public:
        ReadGuard(SrwLock& lock, const Char8* name)
            : mLock(lock)
            , mName(name)
        {
            mLock.AcquireReadLock(mName);
        }

        ~ReadGuard()
        {
            mLock.ReleaseReadLock(mName);
        }

    private:
        SrwLock&        mLock;
        const Char8*    mName;
    };

private:
    SRWLOCK     mLock = SRWLOCK_INIT;
};
