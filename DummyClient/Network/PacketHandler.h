/*    DummyClient/Network/PacketHandler.h    */

#pragma once

#include "DummyClient/Network/Protocol/Packet.h"

/**
 * @class S2C_PacketHandlerMap
 * @brief S2C 패킷을 위한 특수 패킷 핸들러 맵입니다.
 * 
 * 각 특정 패킷 유형에 대한 핸들러를 등록하고 패킷 ID를 기반으로 
 * 적절한 핸들러에 패킷 처리를 위임합니다.
 */
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
    static Bool     Handle_S2C_EnterRoom(SharedPtr<Session> owner, const S2C_EnterRoom& payload);
    static Bool     Handle_S2C_Chat(SharedPtr<Session> owner, const S2C_Chat& payload);
};