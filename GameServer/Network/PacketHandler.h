/*    GameServer/Network/Packet.h    */

#pragma once

#include "GameServer/Network/Protocol/Packet.h"

class C2S_PacketHandlerMap
    : public PacketHandlerMap
{
public:
    static C2S_PacketHandlerMap&    GetInstance();

protected:
    virtual void    RegisterAllHandlers() override;

private:
    C2S_PacketHandlerMap();

    static Bool     Handle_C2S_EnterRoom(SharedPtr<Session> session, const C2S_EnterRoom& payload);
    static Bool     Handle_C2S_Chat(SharedPtr<Session> session, const C2S_Chat& payload);
};

class Room;
extern SharedPtr<Room> gRoom;
