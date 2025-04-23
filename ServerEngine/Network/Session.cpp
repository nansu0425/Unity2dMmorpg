/*    ServerEngine/Network/Session.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Network/Session.h"
#include "ServerEngine/Network/Socket.h"
#include "ServerEngine/Network/Service.h"

Session::Session()
    : mReceiveBuffer(kBufferSize)
{
    ASSERT_CRASH(SUCCESS == SocketUtils::CreateSocket(mSocket), "CREATE_SOCKET_FAILED");
}

Session::~Session()
{
    ASSERT_CRASH(SUCCESS == SocketUtils::CloseSocket(mSocket), "CLOSE_SOCKET_FAILED");
}

Int64 Session::Connect()
{
    return RegisterConnect();
}

void Session::Disconnect(String16 cause)
{
    // 연결 해제 상태로 변경
    if (mIsConnected.exchange(false) == false)
    {
        return;
    }
    // 서비스에서 세션 제거
    GetService()->RemoveSession(GetSharedPtr());

    RegisterDisconnect(std::move(cause));
}

void Session::Send(const Byte* buffer, Int64 numBytes)
{
    // 전송 이벤트 생성
    SendEvent* event = new SendEvent();
    event->owner = GetSharedPtr();
    event->buffer.resize(numBytes);
    ::memcpy(event->buffer.data(), buffer, numBytes);

    RegisterSend(event);
}

HANDLE Session::GetIoObject()
{
    return reinterpret_cast<HANDLE>(mSocket);
}

void Session::DispatchIoEvent(IoEvent* event, Int64 numBytes)
{
    switch (event->type)
    {
    case IoEventType::Connect:
        ProcessConnect();
        break;
    case IoEventType::Disconnect:
        ProcessDisconnect();
        break;
    case IoEventType::Receive:
        ProcessReceive(numBytes);
        break;
    case IoEventType::Send:
        ProcessSend(static_cast<SendEvent*>(event), numBytes);
        break;
    default:
        CRASH("INVALID_EVENT_TYPE");
        break;
    }
}

Int64 Session::RegisterConnect()
{
    ASSERT_CRASH(GetService()->GetType() == ServiceType::Client, "INVALID_SERVICE_TYPE");

    Int64 result = SUCCESS;

    if (IsConnected())
    {
        result = FAILURE;
        return result;
    }

    result = SocketUtils::SetReuseAddress(mSocket, true);
    if (result != SUCCESS)
    {
        return result;
    }

    result = SocketUtils::BindAnyAddress(mSocket, 0);
    if (result != SUCCESS)
    {
        return result;
    }

    mConnectEvent.Init();
    mConnectEvent.owner = shared_from_this();

    // 비동기 연결 요청
    Int64 numBytes = 0;
    result = SocketUtils::ConnectAsync(mSocket, mAddress, OUT &numBytes, &mConnectEvent);

    if (result != SUCCESS)
    {
        if (result == WSA_IO_PENDING)
        {
            // 비동기 작업이 시작되었으므로 성공으로 간주
            result = SUCCESS;
        }
        else
        {
            mConnectEvent.owner.reset();
        }
    }

    return result;
}

Int64 Session::RegisterDisconnect(String16 cause)
{
    mDisconnectEvent.Init();
    mDisconnectEvent.owner = GetSharedPtr();
    mDisconnectEvent.cause = std::move(cause);

    Int64 result = SocketUtils::DisconnectAsync(mSocket, TF_REUSE_SOCKET, &mDisconnectEvent);
    if (result != SUCCESS)
    {
        if (result == WSA_IO_PENDING)
        {
            // 비동기 작업이 시작되었으므로 성공으로 간주
            result = SUCCESS;
        }
        else
        {
            HandleError(result);
            mDisconnectEvent.owner.reset();
        }
    }

    return result;
}

void Session::RegisterReceive()
{
    if (!IsConnected())
    {
        return;
    }

    WSABUF buffer;
    buffer.buf = reinterpret_cast<CHAR*>(mReceiveBuffer.AtWritePos());
    buffer.len = static_cast<Int32>(mReceiveBuffer.GetFreeSize());
    Int64 numBytes = 0;
    Int64 flags = 0;
    mReceiveEvent.Init();
    mReceiveEvent.owner = GetSharedPtr();

    Int64 result = SocketUtils::ReceiveAsync(mSocket, &buffer, OUT &numBytes, OUT &flags, &mReceiveEvent);
    if ((result != SUCCESS) &&
        (result != WSA_IO_PENDING))
    {
        HandleError(result);
        mReceiveEvent.owner.reset();
    }
}

void Session::RegisterSend(SendEvent* event)
{
    if (!IsConnected())
    {
        return;
    }

    WSABUF buffer;
    buffer.buf = reinterpret_cast<CHAR*>(event->buffer.data());
    buffer.len = static_cast<ULONG>(event->buffer.size());
    Int64 numBytes = 0;

    Int64 result = SUCCESS;
    {
        WRITE_GUARD;
        result = SocketUtils::SendAsync(mSocket, &buffer, 1, OUT & numBytes, event);
    }
     
    if ((result != SUCCESS) &&
        (result != WSA_IO_PENDING))
    {
        HandleError(result);
        delete event;
    }
}

void Session::ProcessConnect()
{
    mConnectEvent.owner.reset();

    // 연결 상태로 변경
    mIsConnected.store(true);
    // 서비스에 세션 추가
    GetService()->AddSession(GetSharedPtr());
    // 콘텐츠 코드에서 연결 완료 처리
    OnConnected();
    // 수신 등록
    RegisterReceive();
}

void Session::ProcessDisconnect()
{
    mDisconnectEvent.owner.reset();

    OnDisconnected(std::move(mDisconnectEvent.cause));
}

void Session::ProcessReceive(Int64 numBytes)
{
    mReceiveEvent.owner.reset();

    if (numBytes == 0)
    {
        Disconnect(TEXT_16("Received: 0 bytes"));
        return;
    }

    if (!mReceiveBuffer.OnWritten(numBytes))
    {
        Disconnect(TEXT_16("OnWritten error"));
        return;
    }

    // 콘텐츠 코드에서 수신 처리
    Int64 dataSize = mReceiveBuffer.GetDataSize();
    Int64 numBytesRead = OnReceived(mReceiveBuffer.AtReadPos(), dataSize);
    if ((numBytesRead < 0) ||
        (numBytesRead > dataSize) ||
        (false == mReceiveBuffer.OnRead(numBytesRead)))
    {
        Disconnect(TEXT_16("OnRead error"));
        return;
    }
    mReceiveBuffer.Clear();

    // 수신 완료 후 다시 수신 등록
    RegisterReceive();
}

void Session::ProcessSend(SendEvent* event, Int64 numBytes)
{
    delete event;

    if (numBytes == 0)
    {
        Disconnect(TEXT_16("Sent: 0 bytes"));
        return;
    }

    OnSent(numBytes);
}

void Session::HandleError(Int64 errorCode)
{
    String16 errorMessage = TEXT_16("Error code: ") + std::to_wstring(errorCode);

    switch (errorCode)
    {
    case WSAECONNRESET:
    case WSAECONNABORTED:
        Disconnect(std::move(errorMessage));
        break;
    default:
        std::wcerr << errorMessage << std::endl;
        break;
    }
}
