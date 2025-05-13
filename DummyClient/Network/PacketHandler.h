/*    DummyClient/Network/PacketHandler.h    */

#pragma once

#include "DummyClient/Network/Protocol/Packet.h"

class S2C_PacketHandlerMap
    : public PacketHandlerMap
{
public:
    static S2C_PacketHandlerMap&    GetInstance();

protected:
    virtual void    RegisterAllHandlers() override;

private:
    S2C_PacketHandlerMap();

    static Bool     Handle_S2C_EnterRoom(SharedPtr<Session> session, const S2C_EnterRoom& payload);
    static Bool     Handle_S2C_Chat(SharedPtr<Session> session, const S2C_Chat& payload);
};

class Room;
extern SharedPtr<Room> gRoom;
