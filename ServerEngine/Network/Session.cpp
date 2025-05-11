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

void Session::Disconnect(String8 cause)
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

void Session::Send(SharedPtr<SendBuffer> buffer)
{
    Bool isSending = false;
    // 송신 버퍼 매니저에 버퍼 등록
    {
        WRITE_GUARD;
        mSendBufferMgr.Register(std::move(buffer));
        isSending = mIsSending.exchange(true);
    }

    // 이미 송신 작업 중인 경우
    if (isSending)
    {
        return;
    }

    // 송신 등록은 하나의 스레드만 진입
    RegisterSend_Mgr();
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
        // ProcessSend(numBytes);
        ProcessSend_Mgr(numBytes);
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

    Int64 numBytes = 0;
    mConnectEvent.Init();
    mConnectEvent.owner = shared_from_this();

    result = SocketUtils::ConnectAsync(mSocket, mAddress, OUT &numBytes, &mConnectEvent);
    if ((result != SUCCESS) &&
        (result != WSA_IO_PENDING))
    {
        mConnectEvent.owner.reset();
    }
    else
    {
        result = SUCCESS;
    }

    return result;
}

Int64 Session::RegisterDisconnect(String8 cause)
{
    mDisconnectEvent.Init();
    mDisconnectEvent.owner = GetSharedPtr();
    mDisconnectEvent.cause = std::move(cause);

    Int64 result = SocketUtils::DisconnectAsync(mSocket, TF_REUSE_SOCKET, &mDisconnectEvent);
    if ((result != SUCCESS) &&
        (result != WSA_IO_PENDING))
    {
        HandleError(result);
        mDisconnectEvent.owner.reset();
    }
    else
    {
        result = SUCCESS;
    }

    return result;
}

void Session::RegisterReceive()
{
    // 연결 상태 확인
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

    // 비동기 수신 요청
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
    // 연결 상태 확인
    if (!IsConnected())
    {
        return;
    }

    {
        WRITE_GUARD;
        mSendBuffers.Swap(mSendEvent.buffers);
    }

    Int64 numBytes = 0;
    mSendEvent.Init();
    mSendEvent.owner = GetSharedPtr();

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

void Session::RegisterSend_Mgr()
{
    // 연결 상태 확인
    if (!IsConnected())
    {
        return;
    }

    Int64 numBytes = 0;
    mSendEvent.Init();
    mSendEvent.owner = GetSharedPtr();
    // 송신 이벤트의 버퍼 매니저와 세션의 버퍼 매니저를 교체
    {
        WRITE_GUARD;
        mSendEvent.bufferMgr.Swap(mSendBufferMgr);
    }

    // 비동기 송신 요청
    Int64 result = SocketUtils::SendAsync(mSocket, mSendEvent.bufferMgr.GetWsaBuffers(), mSendEvent.bufferMgr.GetWsaBufferCount(), OUT &numBytes, &mSendEvent);
    if ((result != SUCCESS) &&
        (result != WSA_IO_PENDING))
    {
        HandleError(result);
        mSendEvent.owner.reset();
        mSendBufferMgr.Clear();
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
        RegisterDisconnect(TEXT_8("Failed to add session to service"));
        return;
    }
    // 연결 상태로 변경
    mIsConnected.store(true);
    // 콘텐츠 코드에서 연결 처리
    OnConnected();
    // 수신 시작
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
    // 상대가 정상적으로 연결 종료한 경우
    if (numBytes == 0)
    {
        Disconnect(TEXT_8("Received: 0 bytes"));
        return;
    }
    // 수신 버퍼 쓰기 처리
    if (mReceiveBuffer.OnWritten(numBytes) == false)
    {
        Disconnect(TEXT_8("Receive buffer error"));
        return;
    }
    // 콘텐츠 코드에서 수신 버퍼 데이터 처리
    Int64 processedSize = OnReceived(mReceiveBuffer.AtReadPos(), numBytes);
    if (mReceiveBuffer.OnRead(processedSize) == false)
    {
        Disconnect(TEXT_8("Receive buffer error"));
        return;
    }
    mReceiveBuffer.Clear();
    // 데이터 처리 후 다시 수신 등록
    RegisterReceive();
}

//Int64 Session::ProcessReceiveMessages()
//{
//    Int64 processedSize = 0;
//
//    while (true)
//    {
//        Int64 remainingSize = mReceiveBuffer.GetDataSize() - processedSize;
//        // 메시지 헤더를 포함하는지 확인
//        if (remainingSize < sizeof(MessageHeader))
//        {
//            break;
//        }
//        MessageHeader* header = reinterpret_cast<MessageHeader*>(mReceiveBuffer.AtReadPos() + processedSize);
//        // 메시지 헤더부터 데이터까지 포함하는지 확인
//        if (remainingSize < header->size)
//        {
//            break;
//        }
//        // 콘텐츠 코드에서 수신 메시지 처리
//        OnReceived(ReceiveMessage(header));
//        processedSize += header->size;
//    }
//
//    return processedSize;
//}

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

    // 콘텐츠 코드에서 송신 처리
    OnSent(numBytes);

    { // 송신 버퍼가 없으면 송신 상태를 해제
        WRITE_GUARD;

        if (mSendBuffers.IsEmpty())
        {
            mIsSending.store(false);
            return;
        }
    }

    // 송신 버퍼가 있으면 다시 송신 등록
    RegisterSend();
}

void Session::ProcessSend_Mgr(Int64 numBytes)
{
    mSendEvent.owner.reset();
    mSendEvent.bufferMgr.Clear();
    // 에러가 발생한 경우
    if (mSendEvent.result != SUCCESS)
    {
        HandleError(mSendEvent.result);
        return;
    }

    // 콘텐츠 코드에서 송신 처리
    OnSent(numBytes);

    { 
        WRITE_GUARD;

        // 등록된 송신 버퍼가 없으면 송신 상태를 해제
        if (mSendBufferMgr.IsEmpty())
        {
            mIsSending.store(false);
            return;
        }
    }

    // 송신 버퍼가 있으면 다시 송신 등록
    RegisterSend_Mgr();
}

void Session::HandleError(Int64 errorCode)
{
    switch (errorCode)
    {
    case ERROR_CONNECTION_REFUSED:
        gLogger->Critical(TEXT_8("ERROR_CONNECTION_REFUSED"));
        break;
    case ERROR_NETNAME_DELETED:
        Disconnect(TEXT_8("ERROR_NETNAME_DELETED"));
        break;
    case WSAECONNRESET:
        Disconnect(TEXT_8("WSAECONNRESET"));
        break;
    case WSAECONNABORTED:
        Disconnect(TEXT_8("WSAECONNABORTED"));
        break;
    case WSAENETRESET:
        Disconnect(TEXT_8("WSAENETRESET"));
        break;
    default:
        gLogger->Error(TEXT_8("Error code: {}"), errorCode);
        break;
    }
}
