/*    ServerEngine/Concurrency/Lock.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Concurrency/Deadlock.h"

void RwSpinLock::WriteLock(const Char8* name)
{
#ifdef _DEBUG
    gDeadlockDetector->PushLock(name);
#endif // _DEBUG

    const Int16 writeThreadId = static_cast<Int16>(mLockFlag.load() >> 16);
    // 이미 쓰기 잠금을 한 스레드는 반드시 쓰기 잠금 성공
    if (writeThreadId == tThreadId)
    {
        ++mWriteCount;
        return;
    }

    const UInt64 beginTick = ::GetTickCount64();
    const UInt32 desired = static_cast<UInt32>(tThreadId) << 16;
    while (true)
    {
        // kMaxSpinCount만큼 잠금 시도
        for (UInt32 i = 0; i < kMaxSpinCount; ++i)
        {
            UInt32 expected = kEmptyFlag;
            if (mLockFlag.compare_exchange_weak(OUT expected, desired))
            {
                // 잠금 성공
                ++mWriteCount;
                return;
            }
        }
        // kLockTimeoutTick만큼 시간이 지나도 잠금을 실패하면 크래시
        if (::GetTickCount64() - beginTick > kLockTimeoutTick)
        {
            CRASH("LOCK_TIMEOUT");
        }
        // 다음 스케줄링 때 다시 잠금 시도
        std::this_thread::yield();
    }
}

void RwSpinLock::WriteUnlock(const Char8* name)
{
#ifdef _DEBUG
    gDeadlockDetector->PopLock(name);
#endif // _DEBUG

    // 읽기 -> 쓰기 잠금은 허용하지 않음
    if ((mLockFlag.load() & kReadCountMask) != kEmptyFlag)
    {
        CRASH("INVALID_LOCK_ORDER");
    }

    const Int32 lockCount = --mWriteCount;
    // 모든 쓰기 잠금을 풀었으면 현재 스레드의 쓰기 잠금 해제
    if (lockCount == 0)
    {
        mLockFlag.store(kEmptyFlag);
        return;
    }

    ASSERT_CRASH(lockCount > 0);
}

void RwSpinLock::ReadLock(const Char8* name)
{
#ifdef _DEBUG
    gDeadlockDetector->PushLock(name);
#endif // _DEBUG

    const Int16 writeThreadId = static_cast<Int16>(mLockFlag.load() >> 16);
    // 이미 쓰기 잠금을 한 스레드는 반드시 읽기 잠금 성공
    if (writeThreadId == tThreadId)
    {
        mLockFlag.fetch_add(1);
        return;
    }

    const UInt64 beginTick = ::GetTickCount64();
    while (true)
    {
        // kMaxSpinCount만큼 잠금 시도
        for (UInt32 i = 0; i < kMaxSpinCount; ++i)
        {
            UInt32 expected = (mLockFlag.load() & kReadCountMask);
            if (mLockFlag.compare_exchange_weak(OUT expected, expected + 1))
            {
                // 잠금 성공
                return;
            }
        }
        // kLockTimeoutTick만큼 시간이 지나도 잠금을 실패하면 크래시
        if (::GetTickCount64() - beginTick > kLockTimeoutTick)
        {
            CRASH("LOCK_TIMEOUT");
        }
        // 다음 스케줄링 때 다시 잠금 시도
        std::this_thread::yield();
    }
}

void RwSpinLock::ReadUnlock(const Char8* name)
{
#ifdef _DEBUG
    gDeadlockDetector->PopLock(name);
#endif // _DEBUG

    if ((mLockFlag.fetch_sub(1) & kReadCountMask) == 0)
    {
        CRASH("MULTIPLE_UNLOCK");
    }
}
