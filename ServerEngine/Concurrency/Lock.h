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
    void    WriteLock();
    void    WriteUnlock();
    void    ReadLock();
    void    ReadUnlock();

public:
    class WriteGuard
    {
    public:
        WriteGuard(RwSpinLock& lock) : mLock(lock)  { mLock.WriteLock(); }
        ~WriteGuard()                               { mLock.WriteUnlock(); }

    private:
        RwSpinLock&     mLock;
    };

    class ReadGuard
    {
    public:
        ReadGuard(RwSpinLock& lock) : mLock(lock)   { mLock.ReadLock(); }
        ~ReadGuard()                                { mLock.ReadUnlock(); }

    private:
        RwSpinLock&    mLock;
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
