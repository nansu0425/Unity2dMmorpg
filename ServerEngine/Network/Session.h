/*    ServerEngine/Network/Session.h    */

#pragma once

#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Network/Address.h"
#include "ServerEngine/Io/Event.h"
#include "ServerEngine/Network/Message.h"

class Listener;
class IoEventDispatcher;
class Service;
class SendMessageBuilder;

class Session
    : public IIoObjectOwner
{
public:
    friend class Listener;
    friend class IoEventDispatcher;
    friend class Service;

    enum Constants : Int64
    {
        kBufferSize = 0x0001'0000,
    };

public:
    Session();
    virtual ~Session();

public:     // 외부에서 호출하는 함수
    Int64               Connect();
    void                Disconnect(String16 cause);
    void                Send(SharedPtr<SendMessageBuilder> message);

    SharedPtr<Service>  GetService() const { return mService.lock(); }
    void                SetService(SharedPtr<Service> service) { mService = std::move(service); }
    SOCKET              GetSocket() const { return mSocket; }
    const NetAddress&   GetNetAddress() const { return mAddress; }
    void                SetNetAddress(const NetAddress& address) { mAddress = address; }
    Bool                IsConnected() const { return mIsConnected; }
    SharedPtr<Session>  GetSharedPtr() { return std::static_pointer_cast<Session>(shared_from_this()); }

protected:  // 콘텐츠 코드 인터페이스
    virtual void        OnConnected() = 0;
    virtual void        OnDisconnected(String16 cause) = 0;
    virtual void        OnReceived(ReceiveMessage message) = 0;
    virtual void        OnSent(Int64 numBytes) = 0;

private:    // IIoObjectOwner 인터페이스 구현
    virtual HANDLE      GetIoObject() override;
    virtual void        DispatchIoEvent(IoEvent* event, Int64 numBytes = 0) override;

private:    // 입출력 요청 및 처리
    Int64               RegisterConnect();
    Int64               RegisterDisconnect(String16 cause);
    void                RegisterReceive();
    void                RegisterSend();

    void                ProcessConnect();
    void                ProcessDisconnect();
    void                ProcessReceive(Int64 numBytes);
    Int64               ProcessReceiveMessages();
    void                ProcessSend(Int64 numBytes);

    void                HandleError(Int64 errorCode);

private:
    RW_LOCK;
    WeakPtr<Service>    mService;
    SOCKET              mSocket = INVALID_SOCKET;
    NetAddress          mAddress;
    Atomic<Bool>        mIsConnected = false;
    Atomic<Bool>        mIsSending = false;

private:
    ConnectEvent        mConnectEvent;
    DisconnectEvent     mDisconnectEvent;
    ReceiveEvent        mReceiveEvent;
    SendEvent           mSendEvent;

private:
    ReceiveBuffer       mReceiveBuffer;
    SendBuffers         mSendBuffers;
    // Queue<SharedPtr<SendMessageBuilder>>    mSendQueue;
};
