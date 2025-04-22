/*    ServerEngine/Network/Listener.h    */

#pragma once

#include "ServerEngine/Io/IoManager.h"
#include "ServerEngine/Network/NetAddress.h"

class AcceptEvent;

class Listener
    : public IIoObjectOwner
{
public:
    Listener();
    ~Listener();

public:     // 외부에서 호출하는 함수
    Int64   StartAccept(const NetAddress& address);
    void    CloseSocket();

public:     // IIoObjectOwner 인터페이스 구현
    virtual HANDLE  GetIoObject() override;
    virtual void    DispatchIoEvent(class IoEvent* event, Int64 numBytes = 0) override;

private:
    enum Constants : Int64
    {
        kAcceptCount = 1,
    };

private:    // 내부 처리 함수
    void    RegisterAccept(AcceptEvent* event);
    void    ProcessAccept(AcceptEvent* event, Int64 numBytes);

protected:
    SOCKET          mSocket = INVALID_SOCKET;
    AcceptEvent*    mAcceptEvents = nullptr;
};
