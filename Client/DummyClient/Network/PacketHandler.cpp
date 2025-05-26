/*    DummyClient/Network/PacketHandler.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Network/PacketHandler.h"
#include "GameContent/Chat/Room.h"
#include "GameContent/Common/Player.h"
#include "DummyClient/Network/Session.h"

Bool S2C_PacketHandlerMap::Handle_S2C_EnterRoom(SharedPtr<Session> owner, const S2C_EnterRoom& payload)
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

Bool S2C_PacketHandlerMap::Handle_S2C_Chat(SharedPtr<Session> owner, const S2C_Chat& payload)
{
    gLogger->Debug(TEXT_8("Player[{}]: {}"), std::static_pointer_cast<ServerSession>(owner)->GetPlayerId(), payload.message());

    return true;
}
