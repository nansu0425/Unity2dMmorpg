/*    DummyClient/Packet/Handler.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Packet/Handler.h"
#include "DummyClient/Network/Session.h"
#include "GameLogic/Chat/Room.h"
#include "GameLogic/Common/Player.h"

using namespace core;
using namespace proto;
using namespace game;

namespace dummy
{
    Bool ToClient_PacketHandler::Handle_WorldToClient_EnterRoom(const SharedPtr<core::Session>& owner, const WorldToClient_EnterRoom& payload)
    {
        if (!payload.success())
        {
            gLogger->Error(TEXT_8("Session[{}]: Failed to enter room"), owner->GetId());
            return false;
        }

        if (std::static_pointer_cast<ServerSession>(owner)->GetPlayerId() != payload.id())
        {
            gLogger->Error(TEXT_8("Session[{}]: Player ID mismatch"), owner->GetId());
            return false;
        }

        // 플레이어 찾기
        auto player = PlayerManager::GetInstance().FindPlayer(payload.id());
        if (player == nullptr)
        {
            gLogger->Error(TEXT_8("Session[{}]: Player[{}] not found"), owner->GetId(), payload.id());
            return false;
        }

        // 방 입장
        gRoom->Enter(player);

        return true;
    }

    Bool ToClient_PacketHandler::Handle_WorldToClient_Chat(const SharedPtr<core::Session>& owner, const WorldToClient_Chat& payload)
    {
        return true;
    }
} // namespace dummy
