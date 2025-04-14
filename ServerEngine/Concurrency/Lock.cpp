/*    ServerEngine/Concurrency/Lock.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Concurrency/Deadlock.h"

void RwSpinLock::LockWrite(const Char8* name)
{
#ifdef _DEBUG
    gDeadlockDetector->PushLock(name);
#endif // _DEBUG

    const UInt64 beginTick = ::GetTickCount64();
    Int32 backoff = 1;
    while (true)
    {
        Byte expected = Flag::kEmpty;
        if (mLockFlag.compare_exchange_weak(OUT expected, Flag::kWriteState))
        {
            // 쓰기 잠금 성공
            return;
        }
        // backoff만큼 대기
        for (Int32 i = 0; i < backoff; ++i)
        {
            ::_mm_pause();
        }

        if (backoff < kMaxBackoff)
        {
            // 대기 시간 지수적으로 증가
            backoff <<= 1;
        }
        else
        {
            // 대기 시간이 너무 길어지면 스레드를 양보
            std::this_thread::yield();
            ASSERT_CRASH(::GetTickCount64() - beginTick < kLockTimeoutTick, "LOCK_TIMEOUT");
        }
    }
}

void RwSpinLock::UnlockWrite(const Char8* name)
{
#ifdef _DEBUG
    gDeadlockDetector->PopLock(name);
#endif // _DEBUG

    const Byte prevFlag = mLockFlag.fetch_and(Flag::kEmpty);
    ASSERT_CRASH(prevFlag == Flag::kWriteState, "INVALID_UNLOCK");
}

void RwSpinLock::LockRead(const Char8* name)
{
#ifdef _DEBUG
    gDeadlockDetector->PushLock(name);
#endif // _DEBUG

    const UInt64 beginTick = ::GetTickCount64();
    Int32 backoff = 1;
    while (true)
    {
        Byte expected = (mLockFlag.load() & Flag::kReadCountMask);
        if (mLockFlag.compare_exchange_weak(OUT expected, expected + 1))
        {
            // 읽기 잠금 성공
            return;
        }
        // backoff만큼 대기
        for (Int32 i = 0; i < backoff; ++i)
        {
            ::_mm_pause();
        }

        if (backoff < kMaxBackoff)
        {
            // 대기 시간 지수적으로 증가
            backoff <<= 1;
        }
        else
        {
            // 대기 시간이 너무 길어지면 스레드를 양보
            std::this_thread::yield();
            ASSERT_CRASH(::GetTickCount64() - beginTick < kLockTimeoutTick, "LOCK_TIMEOUT");
        }
    }
}

void RwSpinLock::UnlockRead(const Char8* name)
{
#ifdef _DEBUG
    gDeadlockDetector->PopLock(name);
#endif // _DEBUG

    const Byte prevFlag = mLockFlag.fetch_sub(1);
    ASSERT_CRASH(prevFlag && (prevFlag < kWriteState), "INVALID_UNLOCK");
}
