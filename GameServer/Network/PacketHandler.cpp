/*    GameServer/Network/Packet.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/PacketHandler.h"
#include "GameServer/Network/Session.h"
#include "GameContent/Chat/Room.h"
#include "GameContent/Common/Player.h"

Bool C2S_PacketHandlerMap::Handle_C2S_EnterRoom(SharedPtr<Session> owner, const C2S_EnterRoom& payload)
{
    gRoom->PushJob([owner, id = payload.id()]
                   {
                       // 플레이어 생성 및 매니저에 추가
                       auto player = std::make_shared<Player>(owner, id);
                       std::static_pointer_cast<ClientSession>(owner)->SetPlayerId(id);
                       PlayerManager::GetInstance().AddPlayer(player);

                       // 방 입장
                       gRoom->Enter(std::move(player));

                       // 방 입장 처리 패킷 전송
                       S2C_EnterRoom payload;
                       payload.set_id(id);
                       payload.set_success(true);
                       owner->SendAsync(PacketUtils::MakePacketBuffer(payload, PacketId::S2C_EnterRoom));
                   });


    return true;
}

Bool C2S_PacketHandlerMap::Handle_C2S_Chat(SharedPtr<Session> owner, const C2S_Chat& payload)
{
    gLogger->Info(TEXT_8("Player[{}]: Chat message: {}"), payload.id(), payload.message());

    gRoom->PushJob([owner, id = payload.id(), msg = payload.message()]()
                   {
                       // 룸의 모든 플레이어에게 메시지 전송
                       S2C_Chat payload;
                       payload.set_id(id);
                       payload.set_message(msg);
                       gRoom->Broadcast(PacketUtils::MakePacketBuffer(payload, PacketId::S2C_Chat));
                   });

    return true;
}
