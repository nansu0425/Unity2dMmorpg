/*    DummyClient/Packet/Handler.h    */

#pragma once

#include "Protocol/Packet/Dispatcher.h"

namespace dummy
{
    class ToClient_PacketHandler
        : public proto::PacketDispatcher
    {
    public:
        static ToClient_PacketHandler& GetInstance()
        {
            static ToClient_PacketHandler sInstance;
            return sInstance;
        }

    protected:  // 모든 패킷 핸들러 등록
                        ToClient_PacketHandler() { RegisterAllHandlers(); }

        virtual void    RegisterAllHandlers() override
        {
            using namespace proto;
            
            RegisterHandler<WorldToClient_EnterRoom>(&Handle_WorldToClient_EnterRoom, PacketId::WorldToClient_EnterRoom);
            RegisterHandler<WorldToClient_Chat>(&Handle_WorldToClient_Chat, PacketId::WorldToClient_Chat);
        }

    private:    // 모든 페이로드 핸들러
        static Bool     Handle_WorldToClient_EnterRoom(const SharedPtr<core::Session>& owner, const proto::WorldToClient_EnterRoom& payload);
        static Bool     Handle_WorldToClient_Chat(const SharedPtr<core::Session>& owner, const proto::WorldToClient_Chat& payload);
    };
} // namespace dummy