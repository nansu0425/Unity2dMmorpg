/*    WorldServer/Network/Packet.h    */

#pragma once

#include "Protocol/Packet/Dispatcher.h"

namespace world
{
    class Client2World_PacketDispatcher
        : public proto::PacketDispatcher
    {
    public:
        static Client2World_PacketDispatcher& GetInstance()
        {
            static Client2World_PacketDispatcher sInstance;
            return sInstance;
        }

    protected:  // 모든 패킷 핸들러 등록
                        Client2World_PacketDispatcher() { RegisterAllHandlers(); }

        virtual void    RegisterAllHandlers() override
        {
            using namespace proto;

            RegisterHandler<Client2World_EnterRoom>(&Handle_Client2World_EnterRoom, PacketId::Client2World_EnterRoom);
            RegisterHandler<Client2World_Chat>(&Handle_Client2World_Chat, PacketId::Client2World_Chat);
        }

    private:    // 모든 페이로드 핸들러
        static Bool     Handle_Client2World_EnterRoom(SharedPtr<core::Session> owner, const proto::Client2World_EnterRoom& payload);
        static Bool     Handle_Client2World_Chat(SharedPtr<core::Session> owner, const proto::Client2World_Chat& payload);
    };
} // namespace world
