/*    ServerCore/Common/Tls.cpp    */

#include "ServerCore/Pch.h"

namespace core
{
    thread_local Int32                      tThreadId = 0;
    thread_local Stack<Int32>               tLockStack;
    thread_local SharedPtr<SendChunk>       tSendChunk;
} // namespace core
