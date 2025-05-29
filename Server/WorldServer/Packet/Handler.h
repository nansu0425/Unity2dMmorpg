/*    WorldServer/Packet/Handler.h    */

#pragma once

#include "Protocol/Packet/Dispatcher.h"

namespace world
{
    class ToWorld_PacketHandler
        : public proto::PacketDispatcher
    {
    public:
        static ToWorld_PacketHandler& GetInstance()
        {
            static ToWorld_PacketHandler sInstance;
            return sInstance;
        }

    protected:  // 모든 패킷 핸들러 등록
                        ToWorld_PacketHandler() { RegisterAllHandlers(); }

        virtual void    RegisterAllHandlers() override
        {
            using namespace proto;
            
            RegisterHandler<ClientToWorld_EnterRoom>(&Handle_ClientToWorld_EnterRoom, PacketId::ClientToWorld_EnterRoom);
            RegisterHandler<ClientToWorld_Chat>(&Handle_ClientToWorld_Chat, PacketId::ClientToWorld_Chat);
        }

    private:    // 모든 페이로드 핸들러
        static Bool     Handle_ClientToWorld_EnterRoom(const SharedPtr<core::Session>& owner, const proto::ClientToWorld_EnterRoom& payload);
        static Bool     Handle_ClientToWorld_Chat(const SharedPtr<core::Session>& owner, const proto::ClientToWorld_Chat& payload);
    };
} // namespace world