/*    ServerCore/Common/Macro.h    */

#pragma once

// 외부 변수 값을 바꿀 수 있는 파라미터 위치라는 것을 명시한다
#define OUT

// 함수의 성공 반환 값 (0)
#define SUCCESS     0
// 함수의 실패 반환 값 (-1)
#define FAILURE     -1

// 의도적으로 크래시를 발생시킨다
#define CRASH(cause)                        \
{                                           \
    Int32* crash = nullptr;                 \
    __analysis_assume(crash != nullptr);    \
    *crash = 0;                             \
}                                           \

// 표현식이 참이 아니면 크래시를 발생시킨다
#define ASSERT_CRASH(expr, cause)           \
{                                           \
    if (!(expr))                            \
    {                                       \
        __analysis_assume(!(expr));         \
        CRASH(cause);                       \
    }                                       \
}                                           \

#ifdef _DEBUG
// 표현식이 참이 아니면 크래시를 발생시킨다 (디버그 전용)
#define ASSERT_CRASH_DEBUG(expr, cause)     ASSERT_CRASH(expr, cause)
#else
// 표현식이 참이 아니면 크래시를 발생시킨다 (디버그 전용)
#define ASSERT_CRASH_DEBUG(expr, cause)
#endif // _DEBUG

/*
 * 읽기-쓰기 락 관련 매크로
 *
 * RW_LOCK_ARRAY(count): 지정된 개수만큼의 RwSpinLock 배열을 선언합니다.
 * RW_LOCK: 단일 RwSpinLock을 선언합니다 (RW_LOCK_ARRAY(1)의 축약형).
 *
 * READ_GUARD_IDX(idx): 특정 인덱스의 락에 대한 읽기 가드를 생성합니다.
 *                      RAII 패턴으로 스코프를 벗어날 때 자동으로 락이 해제됩니다.
 * READ_GUARD: 첫 번째 락(인덱스 0)에 대한 읽기 가드를 생성합니다.
 *
 * WRITE_GUARD_IDX(idx): 특정 인덱스의 락에 대한 쓰기 가드를 생성합니다.
 *                       RAII 패턴으로 스코프를 벗어날 때 자동으로 락이 해제됩니다.
 * WRITE_GUARD: 첫 번째 락(인덱스 0)에 대한 쓰기 가드를 생성합니다.
 *
 * 사용 예시:
 * class Example {
 * private:
 *     RW_LOCK; // 클래스 내부에 RwSpinLock mLocks[1]; 선언
 *
 *     void ReadData() {
 *         READ_GUARD // 읽기 락 획득
 *         // 공유 데이터 읽기
 *     } // 스코프 종료 시 자동으로 락 해제
 *
 *     void WriteData() {
 *         WRITE_GUARD // 쓰기 락 획득
 *         // 공유 데이터 수정
 *     } // 스코프 종료 시 자동으로 락 해제
 * };
 */
#define RW_LOCK_ARRAY(count)    RwSpinLock mLocks[count];
#define RW_LOCK                 RW_LOCK_ARRAY(1)
#define READ_GUARD_IDX(idx)     RwSpinLock::ReadGuard readGuard_##idx(mLocks[idx], typeid(*this).name());
#define READ_GUARD              READ_GUARD_IDX(0)
#define WRITE_GUARD_IDX(idx)    RwSpinLock::WriteGuard writeGuard_##idx(mLocks[idx], typeid(*this).name());
#define WRITE_GUARD             WRITE_GUARD_IDX(0)

/*
 * 문자열 관련 매크로
 *
 * TEXT_8(quote): UTF-8 문자열 리터럴로 변환합니다.
 * TEXT_16(quote): 와이드 문자열 리터럴로 변환합니다.
 */
#define TEXT_8(quote)           u8##quote
#define TEXT_16(quote)          L##quote

/*
 * 정수 관련 매크로
 *
 * int16(arg): Int16 타입으로 변환합니다.
 * int32(arg): Int32 타입으로 변환합니다.
 * int64(arg): Int64 타입으로 변환합니다.
 */
#define static_cast_16(arg)     static_cast<Int16>(arg)
#define static_cast_32(arg)     static_cast<Int32>(arg)
#define static_cast_64(arg)     static_cast<Int64>(arg)

/*
 * 크기 관련 매크로
 *
 * sizeof_16(arg): sizeof(arg)의 결과를 Int16 타입으로 캐스팅합니다.
 * sizeof_32(arg): sizeof(arg)의 결과를 Int32 타입으로 캐스팅합니다.
 * sizeof_64(arg): sizeof(arg)의 결과를 Int64 타입으로 캐스팅합니다.
 */
#define sizeof_16(arg)          static_cast_16(sizeof(arg))
#define sizeof_32(arg)          static_cast_32(sizeof(arg))
#define sizeof_64(arg)          static_cast_64(sizeof(arg))

/*
 * 배열 요소 개수 관련 매크로
 *
 * countof_16(array): 배열의 요소 개수를 Int16 타입으로 반환합니다.
 * countof_32(array): 배열의 요소 개수를 Int32 타입으로 반환합니다.
 * countof_64(array): 배열의 요소 개수를 Int64 타입으로 반환합니다.
 */
#define countof_16(array)       static_cast_16(_countof(array))
#define countof_32(array)       static_cast_32(_countof(array))
#define countof_64(array)       static_cast_64(_countof(array))
