/*    GameServer/Network/Packet.h    */

#pragma once

#include "GameServer/Network/Protocol/Packet.h"

class ClientPacketHandlerMap
    : public PacketHandlerMap
{
public:
    static ClientPacketHandlerMap&  GetInstance();

protected:
    virtual void    RegisterAllHandlers() override;

private:
    ClientPacketHandlerMap();

    static Bool     Handle_EnterRoomRequest(SharedPtr<Session> session, const EnterRoomRequest& payload);
    static Bool     Handle_ChatNotify(SharedPtr<Session> session, ChatNotify payload);
};

class Room;
extern SharedPtr<Room> gRoom;
