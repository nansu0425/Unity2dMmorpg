/*    ServerCore/Network/Listener.h    */

#pragma once

#include "ServerCore/Io/Dispatcher.h"
#include "ServerCore/Io/Event.h"
#include "ServerCore/Network/Address.h"

class ServerService;

/**
 * Listener - 네트워크 연결 수신 및 클라이언트 접속 처리 클래스
 *
 * 서버 애플리케이션에서 클라이언트의 연결 요청을 수신하고 처리하는 역할을 담당합니다.
 * Windows의 IOCP 모델을 기반으로 비동기 Accept를 처리하며, 새로운 클라이언트가
 * 연결될 때마다 Session을 ServerService에 전달합니다.
 *
 * 주요 기능:
 * - 소켓 리스닝 초기화 및 설정 (포트 바인딩, 리슨 시작)
 * - 비동기 Accept 처리를 통한 클라이언트 연결 수락
 * - 다중 동시 연결 처리를 위한 AcceptEvent 풀 관리
 * - IO 이벤트 처리를 위한 IIoObjectOwner 인터페이스 구현
 *
 * 사용 패턴:
 * 1. ServerService가 Listener 객체 생성
 * 2. StartAccept() 호출을 통한 리스닝 시작
 * 3. 비동기 Accept 완료 시 이벤트 처리 및 세션 생성
 */
class Listener
    : public IIoObjectOwner
{
public:
    Listener();
    ~Listener();

public:
    Int64               StartAccept(SharedPtr<ServerService> service);

public:
    virtual HANDLE      GetIoObject() override;
    virtual void        DispatchIoEvent(IoEvent* event, Int64 numBytes = 0) override;

private:
    void                RegisterAccept(AcceptEvent* event);
    void                ProcessAccept(AcceptEvent* event, Int64 numBytes);

    void                HandleError(Int64 errorCode);

protected:
    SOCKET                      mSocket = INVALID_SOCKET;
    Vector<AcceptEvent>         mAcceptEvents;
    SharedPtr<ServerService>    mService;
};
