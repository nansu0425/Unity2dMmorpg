/*    ServerEngine/Io/Event.h    */

#pragma once

class Session;
class IIoObjectOwner;

enum class IoEventType : Int64
{
    Connect,
    Accept,
    Recv,
    Send,
};

/*
 * IoManager가 입출력 완료를 전달하기 위해 사용하는 구조체
 */
struct IoEvent
    : public OVERLAPPED
{
    IoEventType                 type;
    SharedPtr<IIoObjectOwner>   owner; // 입출력을 요청한 객체가 입출력 작업이 진행되는 동안 살아있도록 보장한다
    
    void Init() { ::ZeroMemory(this, sizeof(OVERLAPPED)); }

    IoEvent(IoEventType type) : type(type) { Init(); }
};

struct ConnectEvent
    : public IoEvent
{
    ConnectEvent() : IoEvent(IoEventType::Connect) {}
};

struct AcceptEvent
    : public IoEvent
{
    SharedPtr<Session>  session = nullptr;

    AcceptEvent() : IoEvent(IoEventType::Accept) {}
};

struct RecvEvent
    : public IoEvent
{
    RecvEvent() : IoEvent(IoEventType::Recv) {}
};

struct SendEvent
    : public IoEvent
{
    SendEvent() : IoEvent(IoEventType::Send) {}
};
