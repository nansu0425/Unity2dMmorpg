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
    gRoom.Enter(player);

    // 일정 주기마다 채팅 패킷 전송
    C2S_Chat chat;
    chat.set_id(payload.id());
    chat.set_message(TEXT_8("Hello, world!"));
    player->StartSendLoop(PacketUtils::MakePacketBuffer(chat, PacketId::C2S_Chat), 100);

    return true;
}

Bool S2C_PacketHandlerMap::Handle_S2C_Chat(SharedPtr<Session> owner, const S2C_Chat& payload)
{
    return true;
}
