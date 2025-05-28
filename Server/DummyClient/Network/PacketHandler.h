/*    DummyClient/Network/PacketHandler.h    */

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
            RegisterHandler<S2C_EnterRoom>(&Handle_S2C_EnterRoom, proto::PacketId::S2C_EnterRoom);
            RegisterHandler<S2C_Chat>(&Handle_S2C_Chat, proto::PacketId::S2C_Chat);
        }

    private:    // 모든 페이로드 핸들러
        static Bool     Handle_S2C_EnterRoom(SharedPtr<core::Session> owner, const S2C_EnterRoom& payload);
        static Bool     Handle_S2C_Chat(SharedPtr<core::Session> owner, const S2C_Chat& payload);
    };
} // namespace dummy
