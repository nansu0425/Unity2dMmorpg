/*    GameServer/Network/Packet.h    */

#pragma once

#include "GameServer/Network/Protocol/Packet.h"

class C2S_PacketHandlerMap
    : public PacketHandlerMap
{
public:
    static C2S_PacketHandlerMap&    GetInstance()
    {
        static C2S_PacketHandlerMap sInstance;
        return sInstance;
    }

protected:
    C2S_PacketHandlerMap() { RegisterAllHandlers(); }

    virtual void    RegisterAllHandlers() override
    {
        RegisterHandler(
            [this](const Packet& packet)
            {
                return HandlePayload<C2S_EnterRoom>(C2S_PacketHandlerMap::Handle_C2S_EnterRoom, packet);
            },
            PacketId::C2S_EnterRoom);

        RegisterHandler(
            [this](const Packet& packet)
            {
                return HandlePayload<C2S_Chat>(C2S_PacketHandlerMap::Handle_C2S_Chat, packet);
            },
            PacketId::C2S_Chat);
    }

private:    // 모든 페이로드 핸들러
    static Bool     Handle_C2S_EnterRoom(SharedPtr<Session> session, const C2S_EnterRoom& payload);
    static Bool     Handle_C2S_Chat(SharedPtr<Session> session, const C2S_Chat& payload);
};
