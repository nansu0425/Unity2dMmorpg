/*    ServerEngine/Network/Session.h    */

#pragma once

#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Network/Address.h"

struct IoEvent;

class Session
    : public IIoObjectOwner
{
public:
    Session();
    virtual ~Session();

public:     // 외부에서 호출하는 함수
    SOCKET              GetSocket() const { return mSocket; }
    const NetAddress&   GetNetAddress() const { return mAddress; }
    void                SetNetAddress(const NetAddress& address) { mAddress = address; }

public:     // IIoObjectOwner 인터페이스 구현
    virtual HANDLE      GetIoObject() override;
    virtual void        DispatchIoEvent(IoEvent* event, Int64 numBytes = 0) override;

public:
    Byte*           mRecvBuffer[1'000] = {};

private:
    SOCKET          mSocket = INVALID_SOCKET;
    NetAddress      mAddress;
    Atomic<Bool>    mIsConnected = false;
};
