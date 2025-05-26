/*    WorldServer/Network/PacketHandler.h    */

#pragma once

#include "WorldServer/Network/Protocol/Packet.h"

/**
 * @class C2S_PacketHandlerMap
 * @brief C2S 패킷을 위한 특수 패킷 핸들러 맵입니다.
 * 
 * 각 특정 패킷 유형에 대한 핸들러를 등록하고 패킷 ID를 기반으로 
 * 적절한 핸들러에 패킷 처리를 위임합니다.
 */
class C2S_PacketHandlerMap
    : public PacketHandlerMap
{
public:
    static C2S_PacketHandlerMap& GetInstance()
    {
        static C2S_PacketHandlerMap sInstance;
        return sInstance;
    }

protected:  // 모든 패킷 핸들러 등록
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
    static Bool     Handle_C2S_EnterRoom(SharedPtr<Session> owner, const C2S_EnterRoom& payload);
    static Bool     Handle_C2S_Chat(SharedPtr<Session> owner, const C2S_Chat& payload);
};