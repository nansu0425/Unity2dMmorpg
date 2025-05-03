/*    ServerEngine/Network/Session.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Network/Session.h"
#include "ServerEngine/Network/Socket.h"
#include "ServerEngine/Network/Service.h"
#include "ServerEngine/Network/Message.h"

Session::Session()
    : mReceiveBuffer(kReceiveBufferSize)
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
    Int64 result = GetService()->RemoveSession(GetSharedPtr());
    if (SUCCESS != result)
    {
        return;
    }
    // 비동기 연결 해제 등록
    RegisterDisconnect(std::move(cause));
}

void Session::Send(SharedPtr<SendMessageBuilder> message)
{
    Bool isSending = false;

    {
        WRITE_GUARD;
        mSendBuffers.PushMessage(std::move(message));
        isSending = mIsSending.exchange(true);
    }

    // 송신하고 있지 않을 때만 비동기 송신 등록
    if (false == isSending)
    {
        RegisterSend();
    }
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
        ProcessSend(numBytes);
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
            HandleError(result);
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

void Session::RegisterSend()
{
    if (!IsConnected())
    {
        return;
    }

    { // 세션의 버퍼와 이벤트의 버퍼를 스왑
        WRITE_GUARD;
        mSendBuffers.Swap(mSendEvent.buffers);
    }

    Int64 numBytes = 0;
    mSendEvent.Init();
    mSendEvent.owner = GetSharedPtr();

    // 비동기 송신 요청
    Int64 result = SocketUtils::SendAsync(mSocket, mSendEvent.buffers.GetWsaBuffers(), mSendEvent.buffers.GetWsaBufferCount(), OUT &numBytes, &mSendEvent);
    if ((result != SUCCESS) &&
        (result != WSA_IO_PENDING))
    {
        HandleError(result);
        mSendEvent.owner.reset();
        mSendEvent.buffers.Clear();
        mIsSending.store(false);
    }
}

void Session::ProcessConnect()
{
    mConnectEvent.owner.reset();
    // 에러가 발생한 경우
    if (mConnectEvent.result != SUCCESS)
    {
        HandleError(mConnectEvent.result);
        return;
    }
    
    // 서비스에 세션 추가
    Int64 result = GetService()->AddSession(GetSharedPtr());
    if (SUCCESS != result)
    {
        RegisterDisconnect(TEXT_16("Failed to add session to service"));
        return;
    }
    // 연결 상태로 변경
    mIsConnected.store(true);

    // 콘텐츠 코드에서 연결 완료 처리
    OnConnected();
    // 수신 등록
    RegisterReceive();
}

void Session::ProcessDisconnect()
{
    mDisconnectEvent.owner.reset();
    // 에러가 발생한 경우
    if (mDisconnectEvent.result != SUCCESS)
    {
        HandleError(mDisconnectEvent.result);
        return;
    }
    // 콘텐츠 코드에서 연결 해제 처리
    OnDisconnected(std::move(mDisconnectEvent.cause));
}

void Session::ProcessReceive(Int64 numBytes)
{
    mReceiveEvent.owner.reset();
    // 에러가 발생한 경우
    if (mReceiveEvent.result != SUCCESS)
    {
        HandleError(mReceiveEvent.result);
        return;
    }

    if (numBytes == 0)
    {
        Disconnect(TEXT_16("Received: 0 bytes"));
        return;
    }

    if (!mReceiveBuffer.OnWritten(numBytes))
    {
        gLogger->Error(TEXT_16("Failed to write to receive buffer: {} bytes"), numBytes);
        Disconnect(TEXT_16("Receive buffer error"));
        return;
    }

    // 수신 버퍼에 있는 메시지 처리
    Int64 processedSize = ProcessReceiveMessages();
    if ((processedSize < 0) ||
        (processedSize > mReceiveBuffer.GetDataSize()) ||
        (false == mReceiveBuffer.OnRead(processedSize)))
    {
        gLogger->Error(TEXT_16("Failed to read receive buffer.: {} bytes"), numBytes);
        Disconnect(TEXT_16("Receive buffer error"));
        return;
    }
    mReceiveBuffer.Clear();

    // 수신 완료 후 다시 수신 등록
    RegisterReceive();
}

Int64 Session::ProcessReceiveMessages()
{
    Int64 processedSize = 0;

    while (true)
    {
        Int64 remainingSize = mReceiveBuffer.GetDataSize() - processedSize;
        // 메시지 헤더를 포함하는지 확인
        if (remainingSize < sizeof(MessageHeader))
        {
            break;
        }
        MessageHeader* header = reinterpret_cast<MessageHeader*>(mReceiveBuffer.AtReadPos() + processedSize);
        // 메시지 헤더부터 데이터까지 포함하는지 확인
        if (remainingSize < header->size)
        {
            break;
        }
        // 콘텐츠 코드에서 수신 메시지 처리
        OnReceived(ReceiveMessage(header));
        processedSize += header->size;
    }

    return processedSize;
}

void Session::ProcessSend(Int64 numBytes)
{
    mSendEvent.owner.reset();
    mSendEvent.buffers.Clear();

    // 에러가 발생한 경우
    if (mSendEvent.result != SUCCESS)
    {
        HandleError(mSendEvent.result);
        return;
    }

    if (numBytes == 0)
    {
        Disconnect(TEXT_16("Sent: 0 bytes"));
        return;
    }

    // 콘텐츠 코드에서 송신 처리
    OnSent(numBytes);

    { // 송신 버퍼가 비었으면 송신 상태를 해제
        WRITE_GUARD;

        if (mSendBuffers.IsEmpty())
        {
            mIsSending.store(false);
            return;
        }
    }

    // 송신 버퍼에 남아 있는 메시지가 있으면 다시 송신 등록
    RegisterSend();
}

void Session::HandleError(Int64 errorCode)
{
    switch (errorCode)
    {
    case ERROR_CONNECTION_REFUSED:
        gLogger->Critical(TEXT_16("ERROR_CONNECTION_REFUSED"));
        break;
    case ERROR_NETNAME_DELETED:
        Disconnect(TEXT_16("ERROR_NETNAME_DELETED"));
        break;
    case WSAECONNRESET:
        Disconnect(TEXT_16("WSAECONNRESET"));
        break;
    case WSAECONNABORTED:
        Disconnect(TEXT_16("WSAECONNABORTED"));
        break;
    default:
        gLogger->Error(TEXT_16("Error code: {}"), errorCode);
        break;
    }
}
