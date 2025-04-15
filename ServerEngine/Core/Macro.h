/*    ServerEngine/Core/Macro.h    */

#pragma once

#define OUT

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

#define RW_LOCK_ARRAY(count)    RwSpinLock mLocks[count];
#define RW_LOCK                 RW_LOCK_ARRAY(1)
#define READ_GUARD_IDX(idx)     RwSpinLock::ReadGuard readGuard_##idx(mLocks[idx], typeid(*this).name());
#define READ_GUARD              READ_GUARD_IDX(0)
#define WRITE_GUARD_IDX(idx)    RwSpinLock::WriteGuard writeGuard_##idx(mLocks[idx], typeid(*this).name());
#define WRITE_GUARD             WRITE_GUARD_IDX(0)

#ifdef _DEBUG
#define ALLOC_MEMORY(size)      StompAllocator::Alloc(size)
#define FREE_MEMORY(memory)     StompAllocator::Free(memory)
#else
#define ALLOC_MEMORY(size)      BaseAllocator::Alloc(size)
#define FREE_MEMORY(memory)     BaseAllocator::Free(memory)
#endif // _DEBUG

#define TEXT8(quote)            quote
#define TEXT16(quote)           L##quote
