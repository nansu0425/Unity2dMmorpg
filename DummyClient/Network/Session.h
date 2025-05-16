/*    DummyClient/Network/Session.h    */

#include "ServerEngine/Network/Session.h"

#pragma once

class ServerSession
    : public Session
{
public:
    virtual ~ServerSession() override;

    void                SetPlayerId(Int64 id) { mPlayerId = id; }
    Int64               GetPlayerId() const { return mPlayerId; }

protected:
    virtual void        OnConnected() override;
    virtual void        OnDisconnected(String8 cause) override;
    virtual Int64       OnReceived(const Byte* buffer, Int64 numBytes) override;
    virtual void        OnSent(Int64 numBytes) override;

private:
    Int64               mPlayerId = 0;
};

class Room;
extern SharedPtr<Room> gRoom;
