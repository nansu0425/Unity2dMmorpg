/*    ServerEngine/Network/Listener.h    */

#pragma once

#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Io/Event.h"
#include "ServerEngine/Network/Address.h"

class ServerService;

class Listener
    : public IIoObjectOwner
{
public:
    Listener();
    ~Listener();

public:     // 외부에서 호출하는 함수
    Int64   StartAccept(SharedPtr<ServerService> service);

public:     // IIoObjectOwner 인터페이스 구현
    virtual HANDLE  GetIoObject() override;
    virtual void    DispatchIoEvent(IoEvent* event, Int64 numBytes = 0) override;

private:    // 내부 처리 함수
    void    RegisterAccept(AcceptEvent* event);
    void    ProcessAccept(AcceptEvent* event, Int64 numBytes);

    void    HandleError(Int64 errorCode);

protected:
    SOCKET                      mSocket = INVALID_SOCKET;
    Vector<AcceptEvent>         mAcceptEvents;
    SharedPtr<ServerService>    mService;
};
