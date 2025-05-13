/*    DummyClient/Network/PacketHandler.h    */

#pragma once

#include "DummyClient/Network/Protocol/Packet.h"

class S2C_PacketHandlerMap
    : public PacketHandlerMap
{
public:
    static S2C_PacketHandlerMap& GetInstance()
    {
        static S2C_PacketHandlerMap sInstance;
        return sInstance;
    }

protected:  // 모든 패킷 핸들러 등록
    S2C_PacketHandlerMap() { RegisterAllHandlers(); }

    virtual void    RegisterAllHandlers() override
    {
        
        RegisterHandler(
            [this](const Packet& packet)
            {
                return HandlePayload<S2C_EnterRoom>(S2C_PacketHandlerMap::Handle_S2C_EnterRoom, packet);
            },
            PacketId::S2C_EnterRoom);
        
        RegisterHandler(
            [this](const Packet& packet)
            {
                return HandlePayload<S2C_Chat>(S2C_PacketHandlerMap::Handle_S2C_Chat, packet);
            },
            PacketId::S2C_Chat);
        
    }

private:    // 모든 페이로드 핸들러
    static Bool     Handle_S2C_EnterRoom(SharedPtr<Session> session, const S2C_EnterRoom& payload);
    static Bool     Handle_S2C_Chat(SharedPtr<Session> session, const S2C_Chat& payload);
};