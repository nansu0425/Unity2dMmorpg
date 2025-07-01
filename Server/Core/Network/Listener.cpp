/*    Core/Network/Listener.cpp    */

#include "Core/Pch.h"
#include "Core/Io/Event.h"
#include "Core/Network/Listener.h"
#include "Core/Network/Socket.h"
#include "Core/Network/Session.h"
#include "Core/Network/Service.h"

namespace core
{
    /**
     * Listener 생성자
     *
     * 리스너 객체를 초기화합니다. 소켓은 INVALID_SOCKET으로 초기화됩니다.
     * 실제 소켓 생성 및 초기화는 StartAccept() 메서드에서 수행됩니다.
     */
    Listener::Listener()
    {}

    /**
     * Listener 소멸자
     *
     * 리스너 소켓을 안전하게 닫습니다.
     * 소켓이 제대로 닫히지 않으면 ASSERT를 통해 에러를 검출합니다.
     */
    Listener::~Listener()
    {
        ASSERT_CRASH(SUCCESS == SocketUtils::CloseSocket(mSocket), "CLOSE_SOCKET_FAILED");
    }

    /**
     * Accept 작업 시작
     *
     * 서버 서비스와 연결하고 리스닝 소켓을 초기화하여 클라이언트 연결 수락을 시작합니다.
     *
     * 주요 단계:
     * 1. 서버 서비스 설정
     * 2. 리스닝 소켓 생성
     * 3. IO 이벤트 디스패처에 등록
     * 4. 소켓 옵션 설정 (주소 재사용, Linger 등)
     * 5. 주소 바인딩 및 리스닝 시작
     * 6. AcceptEvent 생성 및 비동기 Accept 요청 등록
     *
     * @param service 이 리스너가 사용할 서버 서비스 객체
     * @return 성공 시 SUCCESS(0), 실패 시 FAILURE(-1) 혹은 오류 코드
     */
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

    /**
     * IO 객체 핸들 반환
     *
     * IIoObjectOwner 인터페이스 구현 메서드로,
     * 이 객체가 소유한 소켓의 핸들을 반환합니다.
     * IO 이벤트 디스패처에 소켓 핸들을 등록할 때 사용됩니다.
     *
     * @return 리스닝 소켓의 핸들
     */
    HANDLE Listener::GetIoObject()
    {
        return reinterpret_cast<HANDLE>(mSocket);
    }

    /**
     * IO 이벤트 디스패치 처리
     *
     * IIoObjectOwner 인터페이스 구현 메서드로,
     * IO 완료 이벤트를 처리합니다. 리스너는 Accept 이벤트만 처리합니다.
     *
     * @param event 완료된 IO 이벤트
     * @param numBytes 전송된 바이트 수
     */
    void Listener::DispatchIoEvent(IoEvent* event, Int64 numBytes)
    {
        ASSERT_CRASH_DEBUG(event->type == IoEventType::Accept, "INVALID_EVENT_TYPE");

        AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(event);
        ProcessAccept(acceptEvent, numBytes);
    }

    /**
     * 비동기 Accept 등록
     *
     * 새로운 클라이언트 연결을 수락하기 위한 비동기 Accept 작업을 등록합니다.
     * 서비스로부터 새 세션을 생성하고 AcceptEx API를 사용해 비동기 Accept를 요청합니다.
     *
     * @param event 사용할 AcceptEvent 객체
     */
    void Listener::RegisterAccept(AcceptEvent* event)
    {
        SharedPtr<Session> session = mService->CreateSession();
        Int64 numBytes = 0;
        event->Init();
        event->session = session;
        event->owner = shared_from_this();

        // 비동기 accept 요청
        Int64 result = SocketUtils::AcceptAsync(mSocket, session->GetSocket(), OUT session->mReceiveBuffer.AtWritePos(), OUT & numBytes, event);
        if ((SUCCESS != result) &&
            (WSA_IO_PENDING != result))
        {
            HandleError(result);
            event->owner.reset();
            event->session.reset();
            RegisterAccept(event);
        }
    }

    /**
     * Accept 완료 처리
     *
     * 비동기 Accept가 완료되었을 때 호출되어 새 클라이언트 연결을 처리합니다.
     * 소켓 옵션 업데이트, 피어 주소 획득, 세션 연결 처리 등을 수행합니다.
     *
     * 주요 단계:
     * 1. AcceptEvent에서 세션 및 결과 확인
     * 2. 소켓 옵션 업데이트
     * 3. 클라이언트 주소 정보 획득
     * 4. 세션에 주소 설정 및 연결 처리
     * 5. 다음 Accept 작업 등록
     *
     * @param event 완료된 AcceptEvent 객체
     * @param numBytes 수신된 초기 데이터 크기
     */
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
        Int32 sockAddressLength = sizeof_32(SOCKADDR_IN);
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

    /**
     * 에러 처리
     *
     * 네트워크 작업 중 발생한 에러를 로깅합니다.
     *
     * @param errorCode 발생한 에러 코드
     */
    void Listener::HandleError(Int64 errorCode)
    {
        gLogger->Error(TEXT_8("Error code: {}"), errorCode);
    }
} // namespace core
