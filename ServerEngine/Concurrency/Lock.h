/*    ServerEngine/Concurrency/Lock.h    */

#pragma once

/*
 * 락의 구조: [쓰기 잠금 스레드 Id][읽기 잠금 스레드 개수]
 * 쓰기 잠금: 배타적 잠금
 * 읽기 잠금: 공유 잠금
 * 허용 가능한 재귀 잠금: 쓰기 -> 쓰기, 쓰기 -> 읽기, 읽기 -> 읽기
 */
class RwSpinLock
{
public:
    void    WriteLock(const Char8* name);
    void    WriteUnlock(const Char8* name);
    void    ReadLock(const Char8* name);
    void    ReadUnlock(const Char8* name);

public:
    class WriteGuard
    {
    public:
        WriteGuard(RwSpinLock& lock, const Char8* name)
            : mLock(lock)
            , mName(name)
        {
            mLock.WriteLock(mName);
        }

        ~WriteGuard()
        {
            mLock.WriteUnlock(mName);
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
            mLock.ReadLock(mName);
        }

        ~ReadGuard()
        {
            mLock.ReadUnlock(mName);
        }

    private:
        RwSpinLock&     mLock;
        const Char8*    mName;
    };

private:
    enum : UInt32
    {
        kLockTimeoutTick    = 10'000,
        kMaxSpinCount       = 5'000,
        kWriteThreadMask    = 0xFFFF'0000,
        kReadCountMask      = 0x0000'FFFF,
        kEmptyFlag          = 0x0000'0000,
    };

private:
    Atomic<UInt32>  mLockFlag = kEmptyFlag;
    Int32           mWriteCount = 0;
};
