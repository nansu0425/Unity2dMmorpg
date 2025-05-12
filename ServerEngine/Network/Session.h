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
public:
    friend class Listener;
    friend class IoEventDispatcher;
    friend class Service;

    enum Constants : Int64
    {
        kReceiveBufferSize = 0x0001'0000,
    };

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
    const NetAddress&   GetNetAddress() const { return mAddress; }
    void                SetNetAddress(const NetAddress& address) { mAddress = address; }
    Int64               GetId() const { return mId; }
    void                SetId(Int64 id) { mId = id; }
    Bool                IsConnected() const { return mIsConnected; }
    SharedPtr<Session>  GetSession() { return std::static_pointer_cast<Session>(shared_from_this()); }

protected:  // 콘텐츠 코드 인터페이스
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
