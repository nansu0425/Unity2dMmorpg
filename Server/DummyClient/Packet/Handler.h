/*    DummyClient/Packet/Handler.h    */

#pragma once

#include "Protocol/Packet/Dispatcher.h"

namespace dummy
{
    class S2C_PacketDispatcher
        : public proto::PacketDispatcher
    {
    public:
        static S2C_PacketDispatcher& GetInstance()
        {
            static S2C_PacketDispatcher sInstance;
            return sInstance;
        }

    protected:  // 모든 패킷 핸들러 등록
                        S2C_PacketDispatcher() { RegisterAllHandlers(); }

        virtual void    RegisterAllHandlers() override
        {
            using namespace proto;
            
            RegisterHandler<S2C_EnterRoom>(&Handle_S2C_EnterRoom, PacketId::S2C_EnterRoom);
            RegisterHandler<S2C_Chat>(&Handle_S2C_Chat, PacketId::S2C_Chat);
        }

    private:    // 모든 페이로드 핸들러
        static Bool     Handle_S2C_EnterRoom(const SharedPtr<core::Session>& owner, const proto::S2C_EnterRoom& payload);
        static Bool     Handle_S2C_Chat(const SharedPtr<core::Session>& owner, const proto::S2C_Chat& payload);
    };
} // namespace dummy