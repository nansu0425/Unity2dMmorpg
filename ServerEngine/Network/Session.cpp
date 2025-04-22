/*    ServerEngine/Network/Session.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Network/Session.h"
#include "ServerEngine/Network/SocketUtils.h"

Session::Session()
{
    ASSERT_CRASH_DEBUG(SUCCESS == SocketUtils::CreateSocket(mSocket), "CREATE_SOCKET_FAILED");
}

Session::~Session()
{
    ASSERT_CRASH(SUCCESS == SocketUtils::CloseSocket(mSocket), "CLOSE_SOCKET_FAILED");
}

HANDLE Session::GetIoObject()
{
    return reinterpret_cast<HANDLE>(mSocket);
}

void Session::DispatchIoEvent(IoEvent* event, Int64 numBytes)
{

}
