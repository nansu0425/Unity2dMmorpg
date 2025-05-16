/*    ServerEngine/Network/Listener.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Network/Listener.h"
#include "ServerEngine/Network/Event.h"
#include "ServerEngine/Network/Socket.h"
#include "ServerEngine/Network/Session.h"
#include "ServerEngine/Network/Service.h"

Listener::Listener()
{}

Listener::~Listener()
{
    ASSERT_CRASH(SUCCESS == SocketUtils::CloseSocket(mSocket), "CLOSE_SOCKET_FAILED");
}

Int64 Listener::StartAccept(SharedPtr<ServerService> service)
{
    Int64 result = SUCCESS;

    if (nullptr == (mService = std::move(service)))
    {
        result = FAILURE;
        return result;
    }
    
    // 리슨 소켓 생성
    if (result = SocketUtils::CreateSocket(mSocket))
    {
        return result;
    }
    // 리슨 소켓을 입출력 이벤트 디스패처에 등록
    if (result = mService->GetIoEventDispatcher()->Register(shared_from_this()))
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

    if (result = SocketUtils::BindAddress(mSocket, mService->GetAddress()))
    {
        return result;
    }

    if (result = SocketUtils::Listen(mSocket))
    {
        return result;
    }

    // accept 이벤트 생성 및 등록
    const Int64 acceptCount = mService->GetMaxSessionCount();
    mAcceptEvents.resize(acceptCount);
    for (AcceptEvent& event : mAcceptEvents)
    {
        RegisterAccept(&event);
    }

    return result;
}

HANDLE Listener::GetIoObject()
{
    return reinterpret_cast<HANDLE>(mSocket);
}

void Listener::DispatchIoEvent(IoEvent* event, Int64 numBytes)
{
    ASSERT_CRASH_DEBUG(event->type == IoEventType::Accept, "INVALID_EVENT_TYPE");

    AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(event);
    ProcessAccept(acceptEvent, numBytes);
}

void Listener::RegisterAccept(AcceptEvent* event)
{
    SharedPtr<Session> session = mService->CreateSession();
    Int64 numBytes = 0;
    event->Init();
    event->session = session;
    event->owner = shared_from_this();

    // 비동기 accept 요청
    Int64 result = SocketUtils::AcceptAsync(mSocket, session->GetSocket(), OUT session->mReceiveBuffer.AtWritePos(), OUT &numBytes, event);
    if ((SUCCESS != result) &&
        (WSA_IO_PENDING != result))
    {
        HandleError(result);
        event->owner.reset();
        event->session.reset();
        RegisterAccept(event);
    }
}

void Listener::ProcessAccept(AcceptEvent* event, Int64 numBytes)
{
    event->owner.reset();
    SharedPtr<Session> session = std::move(event->session);
    // accpet 처리 중 에러가 발생한 경우
    if (event->result != SUCCESS)
    {
        HandleError(event->result);
        RegisterAccept(event);
        return;
    }

    Int64 result = SUCCESS;

    result = SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), mSocket);
    if (result != SUCCESS)
    {
        HandleError(result);
        RegisterAccept(event);
        return;
    }

    SOCKADDR_IN sockAddress = {};
    Int32 sockAddressLength = SIZE_32(SOCKADDR_IN);
    // peer 소켓 주소를 가져온다
    result = ::getpeername(session->GetSocket(), OUT reinterpret_cast<SOCKADDR*>(&sockAddress), &sockAddressLength);
    if (result != SUCCESS)
    {
        HandleError(result);
        RegisterAccept(event);
        return;
    }

    // 세션 연결 처리
    session->SetNetAddress(NetAddress(sockAddress));
    session->ProcessConnect();

    RegisterAccept(event);
}

void Listener::HandleError(Int64 errorCode)
{
    gLogger->Error(TEXT_8("Error code: {}"), errorCode);
}
