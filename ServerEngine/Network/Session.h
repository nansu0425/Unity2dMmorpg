/*    ServerEngine/Network/Session.h    */

#pragma once

#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Network/Address.h"
#include "ServerEngine/Io/Event.h"

class Listener;
class IoEventDispatcher;
class Service;

class Session
    : public IIoObjectOwner
{
    friend class Listener;
    friend class IoEventDispatcher;
    friend class Service;

public:
    Session();
    virtual ~Session();

public:     // 외부에서 호출하는 함수
    Int64               Connect();
    void                Disconnect(String16 cause);
    void                Send(const Byte* buffer, Int64 numBytes);

    SharedPtr<Service>  GetService() const { return mService.lock(); }
    void                SetService(SharedPtr<Service> service) { mService = std::move(service); }
    SOCKET              GetSocket() const { return mSocket; }
    const NetAddress&   GetNetAddress() const { return mAddress; }
    void                SetNetAddress(const NetAddress& address) { mAddress = address; }
    Bool                IsConnected() const { return mIsConnected; }
    SharedPtr<Session>  GetSharedPtr() { return std::static_pointer_cast<Session>(shared_from_this()); }

private:    // IIoObjectOwner 인터페이스 구현
    virtual HANDLE      GetIoObject() override;
    virtual void        DispatchIoEvent(IoEvent* event, Int64 numBytes = 0) override;

private:    // 입출력 요청 및 처리
    Int64   RegisterConnect();
    Int64   RegisterDisconnect(String16 cause);
    void    RegisterRecv();
    void    RegisterSend(SendEvent* event);

    void    ProcessConnect();
    void    ProcessDisconnect();
    void    ProcessRecv(Int64 numBytes);
    void    ProcessSend(SendEvent* event, Int64 numBytes);

    void    HandleError(Int64 errorCode);

protected:  // 콘텐츠 코드 인터페이스
    virtual void    OnConnect() = 0;
    virtual void    OnDisconnect(String16 cause) = 0;
    virtual Int64   OnRecv(Byte* buffer, Int64 numBytes) = 0;
    virtual void    OnSend(Int64 numBytes) = 0;

public:
    Byte            mRecvBuffer[1'000] = {};

private:
    RW_LOCK;
    WeakPtr<Service>    mService;
    SOCKET              mSocket = INVALID_SOCKET;
    NetAddress          mAddress;
    Atomic<Bool>        mIsConnected = false;

private:
    ConnectEvent        mConnectEvent;
    DisconnectEvent     mDisconnectEvent;
    RecvEvent           mRecvEvent;
};
