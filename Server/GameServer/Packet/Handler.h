/*    GameServer/Packet/Handler.h    */

#pragma once

#include "Protocol/Packet/Dispatcher.h"

namespace game
{
    class C2S_PacketDispatcher
        : public proto::PacketDispatcher
    {
    public:
        static C2S_PacketDispatcher& GetInstance()
        {
            static C2S_PacketDispatcher sInstance;
            return sInstance;
        }

    protected:  // 모든 패킷 핸들러 등록
                        C2S_PacketDispatcher() { RegisterAllHandlers(); }

        virtual void    RegisterAllHandlers() override
        {
            using namespace proto;
            
            RegisterHandler<C2S_EnterRoom>(&Handle_C2S_EnterRoom, PacketId::C2S_EnterRoom);
            RegisterHandler<C2S_Chat>(&Handle_C2S_Chat, PacketId::C2S_Chat);
        }

    private:    // 모든 페이로드 핸들러
        static Bool     Handle_C2S_EnterRoom(const SharedPtr<core::Session>& owner, const proto::C2S_EnterRoom& payload);
        static Bool     Handle_C2S_Chat(const SharedPtr<core::Session>& owner, const proto::C2S_Chat& payload);
    };
} // namespace game