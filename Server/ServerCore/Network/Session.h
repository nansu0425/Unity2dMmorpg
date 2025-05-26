/*    ServerCore/Network/Session.h    */

#pragma once

#include "ServerCore/Io/Dispatcher.h"
#include "ServerCore/Io/Event.h"
#include "ServerCore/Network/Address.h"

class Listener;
class IoEventDispatcher;
class Service;

/**
 * Session - 네트워크 연결 세션 관리 추상 클래스
 *
 * 단일 네트워크 연결에 대한 전체 수명 주기를 관리하는 추상 클래스입니다.
 * 클라이언트/서버 연결 모두에 사용되며, 비동기 IO 작업(연결, 해제, 데이터 송수신)을
 * Windows IOCP 모델 기반으로 처리합니다.
 *
 * 주요 기능:
 * - 비동기 네트워크 연결 및 해제
 * - 비동기 데이터 송수신
 * - 네트워크 이벤트 처리 (연결, 해제, 수신, 송신)
 * - 오류 상황 감지 및 처리
 * - 송수신 버퍼 관리
 *
 * 사용 패턴:
 * - 상속을 통해 OnConnected, OnDisconnected, OnReceived, OnSent 메서드 구현
 * - 서비스 객체에서 세션 생성 및 관리
 * - 비동기 메서드 호출(ConnectAsync, DisconnectAsync, SendAsync)로 작업 수행
 */
class Session
    : public IIoObjectOwner
{
public:
    friend class Listener;
    friend class IoEventDispatcher;
    friend class Service;

public:
    Session();
    virtual ~Session();

public:     // 외부에서 호출하는 함수
    Int64               ConnectAsync();
    void                DisconnectAsync(String8 cause);
    void                SendAsync(SharedPtr<SendBuffer> buffer);

    SharedPtr<Service>  GetService() const { return mService.lock(); }
    void                SetService(SharedPtr<Service> service) { mService = std::move(service); }
    SOCKET              GetSocket() const { return mSocket; }
    const NetAddress& GetNetAddress() const { return mAddress; }
    void                SetNetAddress(const NetAddress& address) { mAddress = address; }
    Int64               GetId() const { return mId; }
    void                SetId(Int64 id) { mId = id; }
    Bool                IsConnected() const { return mIsConnected; }
    SharedPtr<Session>  GetSession() { return std::static_pointer_cast<Session>(shared_from_this()); }

protected:  // 세션 구현 인터페이스
    virtual void        OnConnected() = 0;
    virtual void        OnDisconnected(String8 cause) = 0;
    virtual Int64       OnReceived(const Byte* buffer, Int64 numBytes) = 0;
    virtual void        OnSent(Int64 numBytes) = 0;

private:    // IIoObjectOwner 인터페이스 구현
    virtual HANDLE      GetIoObject() override;
    virtual void        DispatchIoEvent(IoEvent* event, Int64 numBytes = 0) override;

private:    // 입출력 요청 및 처리
    Int64               RegisterConnect();
    Int64               RegisterDisconnect(String8 cause);
    void                RegisterReceive();
    void                RegisterSend();

    void                ProcessConnect();
    void                ProcessDisconnect();
    void                ProcessReceive(Int64 numBytes);
    void                ProcessSend(Int64 numBytes);

    void                HandleError(Int64 errorCode);

private:
    static constexpr Int64      kReceiveBufferSize = 4096;

private:
    RW_LOCK;
    WeakPtr<Service>    mService;
    SOCKET              mSocket = INVALID_SOCKET;
    NetAddress          mAddress;
    Int64               mId = 0;
    Atomic<Bool>        mIsConnected = false;
    Atomic<Bool>        mIsSending = false;

private:
    ConnectEvent        mConnectEvent;
    DisconnectEvent     mDisconnectEvent;
    ReceiveEvent        mReceiveEvent;
    SendEvent           mSendEvent;

private:
    ReceiveBuffer       mReceiveBuffer;
    SendBufferManager   mSendBufferMgr;
};
