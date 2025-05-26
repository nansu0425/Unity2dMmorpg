/*    WorldServer/Network/Packet.cpp    */

#include "WorldServer/Pch.h"
#include "WorldServer/Network/PacketHandler.h"
#include "WorldServer/Network/Session.h"
#include "GameContent/Chat/Room.h"
#include "GameContent/Common/Player.h"

Bool C2S_PacketHandlerMap::Handle_C2S_EnterRoom(SharedPtr<Session> owner, const C2S_EnterRoom& payload)
{
    // 플레이어 생성 및 매니저에 추가
    auto player = std::make_shared<Player>(owner, payload.id());
    std::static_pointer_cast<ClientSession>(owner)->SetPlayerId(payload.id());
    PlayerManager::GetInstance().AddPlayer(player);

    // 방 입장
    gRoom->Enter(std::move(player));

    // 방 입장 처리 패킷 전송
    S2C_EnterRoom enterRoom;
    enterRoom.set_id(payload.id());
    enterRoom.set_success(true);
    owner->SendAsync(PacketUtils::MakePacketBuffer(enterRoom, PacketId::S2C_EnterRoom));

    return true;
}

Bool C2S_PacketHandlerMap::Handle_C2S_Chat(SharedPtr<Session> owner, const C2S_Chat& payload)
{
    // 룸의 모든 플레이어에게 메시지 전송
    S2C_Chat chat;
    chat.set_id(payload.id());
    chat.set_message(payload.message());
    gRoom->Broadcast(PacketUtils::MakePacketBuffer(chat, PacketId::S2C_Chat), payload.id());

    return true;
}
