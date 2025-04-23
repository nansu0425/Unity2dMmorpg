/*    ServerEngine/Io/Event.h    */

#pragma once

class Session;
class IIoObjectOwner;

enum class IoEventType : Int64
{
    Connect,
    Disconnect,
    Accept,
    Receive,
    Send,
};

/*
 * 입출력 작업 완료를 전달하기 위해 사용하는 구조체
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

struct DisconnectEvent
    : public IoEvent
{
    String16    cause;

    DisconnectEvent() : IoEvent(IoEventType::Disconnect) {}
};

struct AcceptEvent
    : public IoEvent
{
    SharedPtr<Session>  session = nullptr;

    AcceptEvent() : IoEvent(IoEventType::Accept) {}
};

struct ReceiveEvent
    : public IoEvent
{
    ReceiveEvent() : IoEvent(IoEventType::Receive) {}
};

struct SendEvent
    : public IoEvent
{
    Vector<Byte>    buffer; // 전송할 데이터

    SendEvent() : IoEvent(IoEventType::Send) {}
};
