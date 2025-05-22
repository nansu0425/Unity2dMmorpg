/*    ServerEngine/Network/Session.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Network/Session.h"
#include "ServerEngine/Network/Socket.h"
#include "ServerEngine/Network/Service.h"

/**
 * Session 생성자
 *
 * 새로운 세션 객체를 초기화하고 소켓을 생성합니다.
 * 수신 버퍼를 kReceiveBufferSize 크기로 초기화합니다.
 * 소켓 생성 실패 시 크래시가 발생합니다.
 */
Session::Session()
    : mReceiveBuffer(kReceiveBufferSize)
{
    ASSERT_CRASH(SUCCESS == SocketUtils::CreateSocket(mSocket), "CREATE_SOCKET_FAILED");
}

/**
 * Session 소멸자
 *
 * 세션 리소스를 정리하고 소켓을 안전하게 닫습니다.
 * 소켓이 정상적으로 닫히지 않으면 크래시가 발생합니다.
 */
Session::~Session()
{
    ASSERT_CRASH(SUCCESS == SocketUtils::CloseSocket(mSocket), "CLOSE_SOCKET_FAILED");
}

/**
 * 비동기 연결 요청
 *
 * 클라이언트 측에서 서버에 비동기 연결을 시도합니다.
 * 실제 연결 작업은 RegisterConnect 메서드에서 수행됩니다.
 *
 * @return SUCCESS 요청 성공 시, 오류 코드 실패 시
 */
Int64 Session::ConnectAsync()
{
    return RegisterConnect();
}

/**
 * 비동기 연결 해제 요청
 *
 * 세션 연결을 비동기적으로 종료합니다.
 * 이미 연결이 해제된 경우 아무 작업도 수행하지 않습니다.
 *
 * 주요 단계:
 * 1. 연결 상태 플래그 변경
 * 2. 서비스에서 세션 제거
 * 3. 비동기 연결 해제 등록
 *
 * @param cause 연결 해제 원인 메시지
 */
void Session::DisconnectAsync(String8 cause)
{
    // 연결 해제 상태로 변경
    if (mIsConnected.exchange(false) == false)
    {
        return;
    }

    // 서비스에서 세션 제거
    Int64 result = GetService()->RemoveSession(GetSession());
    if (SUCCESS != result)
    {
        return;
    }

    // 비동기 연결 해제 등록
    RegisterDisconnect(std::move(cause));
}

/**
 * 비동기 데이터 송신 요청
 *
 * 제공된 버퍼의 데이터를 비동기적으로 전송합니다.
 * 동시에 여러 스레드가 SendAsync를 호출할 수 있으며,
 * 한 번에 하나의 송신 작업만 진행하도록 보장합니다.
 *
 * @param buffer 전송할 데이터가 포함된 SendBuffer
 */
void Session::SendAsync(SharedPtr<SendBuffer> buffer)
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
    RegisterSend();
}

/**
 * IO 객체 핸들 반환
 *
 * IIoObjectOwner 인터페이스 구현으로, 세션 소켓의 핸들을 반환합니다.
 * IO 이벤트 디스패처에 소켓 핸들을 등록할 때 사용됩니다.
 *
 * @return 세션 소켓의 핸들
 */
HANDLE Session::GetIoObject()
{
    return reinterpret_cast<HANDLE>(mSocket);
}

/**
 * IO 이벤트 디스패치 처리
 *
 * IIoObjectOwner 인터페이스 구현으로, IO 완료 이벤트를 처리합니다.
 * 이벤트 타입에 따라 적절한 처리 메서드로 분기합니다.
 *
 * @param event IO 완료 이벤트
 * @param numBytes 전송된 바이트 수
 */
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

/**
 * 비동기 연결 등록
 *
 * 클라이언트 측에서 서버로의 비동기 연결을 등록합니다.
 * 소켓 옵션 설정, 바인딩 및 비동기 연결 요청을 수행합니다.
 *
 * @return SUCCESS 성공 시, 오류 코드 실패 시
 */
Int64 Session::RegisterConnect()
{
    ASSERT_CRASH(GetService()->GetType() == ServiceType::Client, "INVALID_SERVICE_TYPE");

    Int64 result = SUCCESS;

    // 연결 상태 확인
    if (IsConnected())
    {
        result = FAILURE;
        return result;
    }

    // 소켓 주소 재사용 허용
    result = SocketUtils::SetReuseAddress(mSocket, true);
    if (result != SUCCESS)
    {
        return result;
    }

    // 주소 임의 설정
    result = SocketUtils::BindAnyAddress(mSocket, 0);
    if (result != SUCCESS)
    {
        return result;
    }

    Int64 numBytes = 0;
    mConnectEvent.Init();
    mConnectEvent.owner = shared_from_this();

    // 비동기 연결 요청
    result = SocketUtils::ConnectAsync(mSocket, mAddress, OUT & numBytes, &mConnectEvent);
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

/**
 * 비동기 연결 해제 등록
 *
 * 세션의 연결을 비동기적으로 해제합니다.
 * DisconnectEx API를 사용하여 연결 해제 요청을 등록합니다.
 *
 * @param cause 연결 해제 원인 메시지
 * @return SUCCESS 성공 시, 오류 코드 실패 시
 */
Int64 Session::RegisterDisconnect(String8 cause)
{
    mDisconnectEvent.Init();
    mDisconnectEvent.owner = GetSession();
    mDisconnectEvent.cause = std::move(cause);

    // 비동기 연결 해제 요청
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

/**
 * 비동기 데이터 수신 등록
 *
 * 소켓으로부터 데이터를 비동기적으로 수신하기 위한 작업을 등록합니다.
 * WSARecv API를 사용하여 비동기 수신 요청을 등록합니다.
 */
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
    mReceiveEvent.owner = GetSession();

    // 비동기 수신 요청
    Int64 result = SocketUtils::ReceiveAsync(mSocket, &buffer, OUT & numBytes, OUT & flags, &mReceiveEvent);
    if ((result != SUCCESS) &&
        (result != WSA_IO_PENDING))
    {
        HandleError(result);
        mReceiveEvent.owner.reset();
    }
}

/**
 * 비동기 데이터 송신 등록
 *
 * 세션의 송신 버퍼에 있는 데이터를 비동기적으로 전송하기 위한 작업을 등록합니다.
 * WSASend API를 사용하여 비동기 송신 요청을 등록합니다.
 */
void Session::RegisterSend()
{
    // 연결 상태 확인
    if (!IsConnected())
    {
        return;
    }

    Int64 numBytes = 0;
    mSendEvent.Init();
    mSendEvent.owner = GetSession();
    // 송신 이벤트의 버퍼 매니저와 세션의 버퍼 매니저를 교체
    {
        WRITE_GUARD;
        mSendEvent.bufferMgr.Swap(mSendBufferMgr);
    }

    // 비동기 송신 요청
    Int64 result = SocketUtils::SendAsync(mSocket, mSendEvent.bufferMgr.GetWsaBuffers(), mSendEvent.bufferMgr.GetWsaBufferCount(), OUT & numBytes, &mSendEvent);
    if ((result != SUCCESS) &&
        (result != WSA_IO_PENDING))
    {
        HandleError(result);
        mSendEvent.owner.reset();
        mSendBufferMgr.Clear();
        mIsSending.store(false);
    }
}

/**
 * 연결 완료 처리
 *
 * 비동기 연결 요청이 완료되었을 때 호출됩니다.
 * 연결 결과를 확인하고, 성공 시 세션을 서비스에 추가하고 수신을 시작합니다.
 */
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
    Int64 result = GetService()->AddSession(GetSession());
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

/**
 * 연결 해제 완료 처리
 *
 * 비동기 연결 해제 요청이 완료되었을 때 호출됩니다.
 * 연결 해제 결과를 확인하고, 파생 클래스의 OnDisconnected 메서드를 호출합니다.
 */
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

/**
 * 데이터 수신 완료 처리
 *
 * 비동기 데이터 수신이 완료되었을 때 호출됩니다.
 * 수신 결과를 확인하고, 성공 시 데이터를 처리하고 다음 수신을 등록합니다.
 *
 * @param numBytes 수신된 바이트 수
 */
void Session::ProcessReceive(Int64 numBytes)
{
    mReceiveEvent.owner.reset();

    // 에러가 발생한 경우
    if (mReceiveEvent.result != SUCCESS)
    {
        HandleError(mReceiveEvent.result);
        return;
    }

    // 상대가 우아한 연결 해제를 한 경우
    if (numBytes == 0)
    {
        DisconnectAsync(TEXT_8("Graceful closure"));
        return;
    }

    // 수신 버퍼 쓰기 처리
    if (mReceiveBuffer.OnWritten(numBytes) == false)
    {
        DisconnectAsync(TEXT_8("Receive buffer error"));
        return;
    }

    // 콘텐츠 코드에서 수신 처리
    Int64 processedSize = OnReceived(mReceiveBuffer.AtReadPos(), numBytes);
    if (mReceiveBuffer.OnRead(processedSize) == false)
    {
        DisconnectAsync(TEXT_8("Receive buffer error"));
        return;
    }
    mReceiveBuffer.Clear();

    // 데이터 처리 후 다시 수신 등록
    RegisterReceive();
}

/**
 * 데이터 송신 완료 처리
 *
 * 비동기 데이터 송신이 완료되었을 때 호출됩니다.
 * 송신 결과를 확인하고, 성공 시 파생 클래스의 OnSent 메서드를 호출합니다.
 * 송신 버퍼에 더 데이터가 있으면 다음 송신을 등록합니다.
 *
 * @param numBytes 송신된 바이트 수
 */
void Session::ProcessSend(Int64 numBytes)
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
    RegisterSend();
}

/**
 * 오류 처리
 *
 * 네트워크 작업 중 발생한 오류를 처리합니다.
 * 오류 코드에 따라 적절한 로깅과 연결 해제 등의 작업을 수행합니다.
 *
 * @param errorCode 발생한 오류 코드
 */
void Session::HandleError(Int64 errorCode)
{
    switch (errorCode)
    {
    case ERROR_CONNECTION_REFUSED:
        gLogger->Critical(TEXT_8("ERROR_CONNECTION_REFUSED"));
        break;
    case ERROR_NETNAME_DELETED:
        DisconnectAsync(TEXT_8("ERROR_NETNAME_DELETED"));
        break;
    case WSAECONNRESET:
        DisconnectAsync(TEXT_8("WSAECONNRESET"));
        break;
    case WSAECONNABORTED:
        DisconnectAsync(TEXT_8("WSAECONNABORTED"));
        break;
    case WSAENETRESET:
        DisconnectAsync(TEXT_8("WSAENETRESET"));
        break;
    case WSAENOTCONN:
        DisconnectAsync(TEXT_8("WSAENOTCONN"));
        break;
    default:
        gLogger->Error(TEXT_8("Error code: {}"), errorCode);
        break;
    }
}
