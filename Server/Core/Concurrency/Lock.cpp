/*    Core/Concurrency/Lock.cpp    */

#include "Core/Pch.h"
#include "Core/Concurrency/Deadlock.h"

namespace core
{
    /**
     * 쓰기 잠금을 획득합니다.
     *
     * @param name 락 식별자 (디버깅용)
     *
     * 동작:
     * 1. 디버그 모드에서 데드락 감지기에 락 획득 정보 기록
     * 2. 원자적으로 플래그를 쓰기 상태로 변경 시도
     * 3. 획득 실패 시 지수적 백오프로 재시도
     * 4. 설정 시간 초과시 크래시 발생
     */
    void RwSpinLock::LockWrite(const Char8* name)
    {
#ifdef _DEBUG
        gDeadlockDetector->PushLock(name);
#endif // _DEBUG

        const UInt64 beginTick = ::GetTickCount64();
        Int32 backoff = 1;
        while (true)
        {
            UInt64 expected = kEmptyFlag;
            if (mLockFlag.compare_exchange_weak(OUT expected, kWriteFlag))
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
                ASSERT_CRASH(::GetTickCount64() - beginTick < kLockTimeoutMs, "LOCK_TIMEOUT");
            }
        }
    }

    /**
     * 쓰기 잠금을 해제합니다.
     *
     * @param name 락 식별자 (디버깅용)
     *
     * 동작:
     * 1. 디버그 모드에서 데드락 감지기에서 락 해제 정보 기록
     * 2. 원자적으로 플래그를 빈 상태로 변경
     * 3. 이전 상태가 쓰기 상태가 아니면 크래시 발생
     */
    void RwSpinLock::UnlockWrite(const Char8* name)
    {
#ifdef _DEBUG
        gDeadlockDetector->PopLock(name);
#endif // _DEBUG

        const UInt64 prevFlag = mLockFlag.fetch_and(kEmptyFlag);
        ASSERT_CRASH_DEBUG(prevFlag == kWriteFlag, "INVALID_UNLOCK");
    }

    /**
     * 읽기 잠금을 획득합니다.
     *
     * @param name 락 식별자 (디버깅용)
     *
     * 동작:
     * 1. 디버그 모드에서 데드락 감지기에 락 획득 정보 기록
     * 2. 원자적으로 읽기 카운트를 증가 시도
     * 3. 획득 실패 시 지수적 백오프로 재시도
     * 4. 설정 시간 초과시 크래시 발생
     */
    void RwSpinLock::LockRead(const Char8* name)
    {
#ifdef _DEBUG
        gDeadlockDetector->PushLock(name);
#endif // _DEBUG

        const UInt64 beginTick = ::GetTickCount64();
        Int32 backoff = 1;
        while (true)
        {
            UInt64 expected = (mLockFlag.load() & kReadCountMask);
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
                ASSERT_CRASH(::GetTickCount64() - beginTick < kLockTimeoutMs, "LOCK_TIMEOUT");
            }
        }
    }

    /**
     * 읽기 잠금을 해제합니다.
     *
     * @param name 락 식별자 (디버깅용)
     *
     * 동작:
     * 1. 디버그 모드에서 데드락 감지기에서 락 해제 정보 기록
     * 2. 원자적으로 읽기 카운트를 감소
     * 3. 이전 상태가 유효하지 않으면 크래시 발생
     */
    void RwSpinLock::UnlockRead(const Char8* name)
    {
#ifdef _DEBUG
        gDeadlockDetector->PopLock(name);
#endif // _DEBUG

        const UInt64 prevFlag = mLockFlag.fetch_sub(1);
        ASSERT_CRASH_DEBUG(prevFlag && (prevFlag < kWriteFlag), "INVALID_UNLOCK");
    }
} // namespace core
