/*    DummyClient/Network/Packet.h    */

#pragma once

#include "Protocol/Packet/Dispatcher.h"

namespace dummy
{
    class World2Client_PacketDispatcher
        : public proto::PacketDispatcher
    {
    public:
        static World2Client_PacketDispatcher& GetInstance()
        {
            static World2Client_PacketDispatcher sInstance;
            return sInstance;
        }

    protected:  // 모든 패킷 핸들러 등록
                        World2Client_PacketDispatcher() { RegisterAllHandlers(); }

        virtual void    RegisterAllHandlers() override
        {
            using namespace proto;

            RegisterHandler<World2Client_EnterRoom>(&Handle_World2Client_EnterRoom, PacketId::World2Client_EnterRoom);
            RegisterHandler<World2Client_Chat>(&Handle_World2Client_Chat, PacketId::World2Client_Chat);
        }

    private:    // 모든 페이로드 핸들러
        static Bool     Handle_World2Client_EnterRoom(SharedPtr<core::Session> owner, const proto::World2Client_EnterRoom& payload);
        static Bool     Handle_World2Client_Chat(SharedPtr<core::Session> owner, const proto::World2Client_Chat& payload);
    };
} // namespace dummy
