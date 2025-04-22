/*    ServerEngine/Io/IoEvent.h    */

#pragma once

class Session;

enum class IoEventType : Int64
{
    Connect,
    Accept,
    Recv,
    Send,
};

class IoEvent
    : public OVERLAPPED
{
public:
    IoEvent(IoEventType type);

    void            Init();
    IoEventType     GetType() const { return mType; }

protected:
    IoEventType     mType;
};

class ConnectEvent
    : public IoEvent
{
public:
    ConnectEvent() : IoEvent(IoEventType::Connect) {}
};

class AcceptEvent
    : public IoEvent
{
public:
    AcceptEvent() : IoEvent(IoEventType::Accept) {}

    Session*    GetSession() const { return mSession; }
    void        SetSession(Session* session) { mSession = session; }

private:
    Session*    mSession = nullptr;
};

class RecvEvent
    : public IoEvent
{
public:
    RecvEvent() : IoEvent(IoEventType::Recv) {}
};

class SendEvent
    : public IoEvent
{
public:
    SendEvent() : IoEvent(IoEventType::Send) {}
};
