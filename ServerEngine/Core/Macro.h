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
#define ASSERT_CRASH(expr)                  \
{                                           \
    if (!(expr))                            \
    {                                       \
        __analysis_assume(!(expr));         \
        CRASH("ASSERT_CRASH");              \
    }                                       \
}                                           \
