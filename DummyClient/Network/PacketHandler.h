/*    DummyClient/Network/PacketHandler.h    */

#pragma once

#include "DummyClient/Network/Protocol/Packet.h"

class ServerPacketHandlerMap
    : public PacketHandlerMap
{
public:
    static ServerPacketHandlerMap&  GetInstance();

protected:
    virtual void    RegisterAllHandlers() override;

private:
    ServerPacketHandlerMap();

    static Bool     Handle_EnterRoomResponse(SharedPtr<Session> session, EnterRoomResponse payload);
    static Bool     Handle_ChatBroadcast(SharedPtr<Session> session, ChatBroadcast payload);
};

class Room;
extern SharedPtr<Room> gRoom;
