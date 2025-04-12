/*    ServerEngine/Concurrency/Lock.h    */

#pragma once

/*
 * 락의 구조: [쓰기 잠금 스레드 Id][읽기 잠금 스레드 개수]
 * 쓰기 잠금: 배타적 잠금
 * 읽기 잠금: 공유 잠금
 */
class RwSpinlock
{
private:
    enum : Uint32
    {
        kLockTimeoutTick    = 10'000,
        kMaxSpinCount       = 5'000,
        kWriteThreadMask    = 0xFFFF'0000,
        kReadCountMask      = 0x0000'FFFF,
        kEmptyFlag          = 0x0000'0000,
    };

public:
    void    WriteLock();
    void    WriteUnlock();
    void    ReadLock();
    void    ReadUnlock();

private:
    Atomic<Uint32>  mLockFlag = kEmptyFlag;
    Int32           mWriteCount = 0;
};
