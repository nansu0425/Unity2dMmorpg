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
