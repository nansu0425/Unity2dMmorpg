/*    Core/Common/Tls.h    */

#pragma once

namespace core
{
    class SendChunk;

    extern thread_local Int32                       tThreadId;
    extern thread_local Stack<Int32>                tLockStack;
    extern thread_local SharedPtr<SendChunk>        tSendChunk;
} // namespace core
