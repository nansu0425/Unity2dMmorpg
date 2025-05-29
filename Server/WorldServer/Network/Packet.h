/*    WorldServer/Network/Packet.h    */

#pragma once

#include "Protocol/Packet/Dispatcher.h"

namespace world
{
    class ToWorld_PacketDispatcher
        : public proto::PacketDispatcher
    {
    public:
        static ToWorld_PacketDispatcher& GetInstance()
        {
            static ToWorld_PacketDispatcher sInstance;
            return sInstance;
        }

    protected:  // 모든 패킷 핸들러 등록
                        ToWorld_PacketDispatcher() { RegisterAllHandlers(); }

        virtual void    RegisterAllHandlers() override
        {
            using namespace proto;

            RegisterHandler<ClientToWorld_EnterRoom>(&Handle_ClientToWorld_EnterRoom, PacketId::ClientToWorld_EnterRoom);
            RegisterHandler<ClientToWorld_Chat>(&Handle_ClientToWorld_Chat, PacketId::ClientToWorld_Chat);
        }

    private:    // 모든 페이로드 핸들러
        static Bool     Handle_ClientToWorld_EnterRoom(SharedPtr<core::Session> owner, const proto::ClientToWorld_EnterRoom& payload);
        static Bool     Handle_ClientToWorld_Chat(SharedPtr<core::Session> owner, const proto::ClientToWorld_Chat& payload);
    };
} // namespace world
