/*    GameServer/Packet/Handler.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Packet/Handler.h"
#include "GameServer/Network/Session.h"
#include "Protocol/Packet/Utils.h"
#include "GameServer/Chat/Room.h"
#include "GameServer/Entity/Player.h"

namespace game
{
    Bool ToWorld_PacketHandler::Handle_ClientToWorld_EnterRoom(const SharedPtr<core::Session>& owner, const proto::ClientToWorld_EnterRoom& payload)
    {
        // 플레이어 생성 및 매니저에 추가
        auto player = std::make_shared<Player>(owner, payload.id());
        std::static_pointer_cast<ClientSession>(owner)->SetPlayerId(payload.id());
        PlayerManager::GetInstance().AddPlayer(player);

        // 방 입장
        gRoom->Enter(std::move(player));

        // 방 입장 처리 패킷 전송
        proto::WorldToClient_EnterRoom enterRoom;
        enterRoom.set_id(payload.id());
        enterRoom.set_success(true);
        proto::PacketUtils::Send(owner, enterRoom);

        return true;
    }

    Bool ToWorld_PacketHandler::Handle_ClientToWorld_Chat(const SharedPtr<core::Session>& owner, const proto::ClientToWorld_Chat& payload)
    {
        // 룸의 모든 플레이어에게 메시지 전송
        proto::WorldToClient_Chat chat;
        chat.set_id(payload.id());
        chat.set_message(payload.message());
        gRoom->Broadcast(proto::PacketUtils::MakeSendBuffer(chat, proto::PacketId::WorldToClient_Chat), payload.id());

        return true;
    }
} // namespace game
