/*    ServerEngine/Network/Listener.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Network/Listener.h"
#include "ServerEngine/Io/IoEvent.h"
#include "ServerEngine/Network/SocketUtils.h"
#include "ServerEngine/Network/Session.h"

Listener::Listener()
{
    mAcceptEvents = new AcceptEvent[kAcceptCount];
}

Listener::~Listener()
{
    CloseSocket();

    delete[] mAcceptEvents;
    mAcceptEvents = nullptr;
}

Int64 Listener::StartAccept(const NetAddress& address)
{
    Int64 result = SUCCESS;
    // 리슨 소켓 생성
    if (result = SocketUtils::CreateSocket(mSocket))
    {
        return result;
    }
    // 리슨 소켓을 io 매니저에 등록
    if (result = gIoManager.RegisterIoObject(this))
    {
        return result;
    }

    if (result = SocketUtils::SetReuseAddress(mSocket, true))
    {
        return result;
    }

    if (result = SocketUtils::SetLinger(mSocket, 0, 0))
    {
        return result;
    }

    if (result = SocketUtils::BindAddress(mSocket, address))
    {
        return result;
    }

    if (result = SocketUtils::Listen(mSocket))
    {
        return result;
    }

    // accept 이벤트 등록
    for (Int64 i = 0; i < kAcceptCount; ++i)
    {
        RegisterAccept(&mAcceptEvents[i]);
    }

    return result;
}

void Listener::CloseSocket()
{
    ASSERT_CRASH(SUCCESS == SocketUtils::CloseSocket(mSocket), "CLOSE_SOCKET_FAILED");
}

HANDLE Listener::GetIoObject()
{
    return reinterpret_cast<HANDLE>(mSocket);
}

void Listener::DispatchIoEvent(IoEvent* event, Int64 numBytes)
{
    ASSERT_CRASH_DEBUG(event->GetType() == IoEventType::Accept, "INVALID_EVENT_TYPE");

    AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(event);
    ProcessAccept(acceptEvent, numBytes);
}

void Listener::RegisterAccept(AcceptEvent* event)
{
    Session* session = new Session();
    event->Init();
    event->SetSession(session);

    Int64 numBytes = 0;
    Int64 result = SUCCESS;
    // 성공적으로 비동기 작업이 시작되지 않은 경우
    if (WSA_IO_PENDING != (result = SocketUtils::AcceptAsync(mSocket, session, OUT numBytes, event)))
    {
        std::cout << "AcceptAsync failed: " << result << std::endl;
        RegisterAccept(event);
    }
}

void Listener::ProcessAccept(AcceptEvent* event, Int64 numBytes)
{
    Session* session = event->GetSession();
    event->SetSession(nullptr);
    Int64 result = SUCCESS;

    if (result = SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), mSocket))
    {
        std::cout << "SetUpdateAcceptSocket failed: " << result << std::endl;
        delete session;
        RegisterAccept(event);
        return;
    }

    SOCKADDR_IN sockAddress = {};
    Int32 sockAddressLength = SIZE_32(SOCKADDR_IN);

    if (result = ::getpeername(session->GetSocket(), OUT reinterpret_cast<SOCKADDR*>(&sockAddress), &sockAddressLength))
    {
        std::cout << "getpeername failed: " << result << std::endl;
        delete session;
        RegisterAccept(event);
        return;
    }

    session->SetNetAddress(NetAddress(sockAddress));

    std::cout << "Client connected!" << std::endl;
    delete session;

    RegisterAccept(event);
}
