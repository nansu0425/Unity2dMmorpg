/*    ServerEngine/Concurrency/Lock.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Concurrency/Lock.h"

void RwSpinlock::WriteLock()
{
    const Int16 writeThreadId = static_cast<Int16>(mLockFlag.load() >> 16);
    // 쓰기 잠금을 한 스레드는 반드시 다시 쓰기 잠금 성공
    if (writeThreadId == tThreadId)
    {
        ++mWriteCount;
        return;
    }

    const Uint64 beginTick = ::GetTickCount64();
    const Uint32 desired = static_cast<Uint32>(tThreadId) << 16;
    while (true)
    {
        // kSpinCount만큼 잠금 시도
        for (Uint32 i = 0; i < kMaxSpinCount; ++i)
        {
            Uint32 expected = kEmptyFlag;
            // 읽기/쓰기 잠금이 없는 상태면 잠금 성공
            if (mLockFlag.compare_exchange_strong(OUT expected, desired))
            {
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

void RwSpinlock::WriteUnlock()
{}

void RwSpinlock::ReadLock()
{}

void RwSpinlock::ReadUnlock()
{}
