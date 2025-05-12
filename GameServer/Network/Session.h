/*    GameServer/Network/Session.h    */

#pragma once

#include "ServerEngine/Network/Session.h"
#include "GameServer/Network/Protocol/Packet.pb.h"

class Room;

extern SharedPtr<Room>  gRoom;

class ClientSession
    : public Session
{
protected:
    virtual void        OnConnected() override;
    virtual void        OnDisconnected(String8 cause) override;
    virtual Int64       OnReceived(const Byte* buffer, Int64 numBytes) override;
    virtual void        OnSent(Int64 numBytes) override;
};
