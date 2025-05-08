/*    ServerEngine/Common/Macro.h    */

#pragma once

#define OUT
#define SUCCESS     0
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

#define RW_LOCK_ARRAY(count)    RwSpinLock mLocks[count];
#define RW_LOCK                 RW_LOCK_ARRAY(1)
#define READ_GUARD_IDX(idx)     RwSpinLock::ReadGuard readGuard_##idx(mLocks[idx], typeid(*this).name());
#define READ_GUARD              READ_GUARD_IDX(0)
#define WRITE_GUARD_IDX(idx)    RwSpinLock::WriteGuard writeGuard_##idx(mLocks[idx], typeid(*this).name());
#define WRITE_GUARD             WRITE_GUARD_IDX(0)

#define USE_STOMP_ALLOCATOR     false

#define TEXT_8(quote)           u8##quote
#define TEXT_16(quote)          L##quote

#define SIZE_16(arg)            static_cast<Int16>(sizeof(arg))
#define SIZE_32(arg)            static_cast<Int32>(sizeof(arg))
#define SIZE_64(arg)            static_cast<Int64>(sizeof(arg))
#define NUM_ELEM_16(array)      static_cast<Int16>(sizeof(array) / sizeof(array[0]))
#define NUM_ELEM_32(array)      static_cast<Int32>(sizeof(array) / sizeof(array[0]))
#define NUM_ELEM_64(array)      static_cast<Int64>(sizeof(array) / sizeof(array[0]))

#define MESSAGE_ID(id)          static_cast<MessageId>(id)
