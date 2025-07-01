/*    WorldServer/Packet/Handler.cpp    */

#include "WorldServer/Pch.h"
#include "WorldServer/Packet/Handler.h"
#include "WorldServer/Network/Session.h"
#include "Protocol/Packet/Utils.h"
#include "GameLogic/Chat/Room.h"
#include "GameLogic/Entity/Player.h"

using namespace core;
using namespace proto;
using namespace game;

namespace world
{
    Bool ToWorld_PacketHandler::Handle_ClientToWorld_EnterRoom(const SharedPtr<core::Session>& owner, const ClientToWorld_EnterRoom& payload)
    {
        // 플레이어 생성 및 매니저에 추가
        auto player = std::make_shared<Player>(owner, payload.id());
        std::static_pointer_cast<ClientSession>(owner)->SetPlayerId(payload.id());
        PlayerManager::GetInstance().AddPlayer(player);

        // 방 입장
        gRoom->Enter(std::move(player));

        // 방 입장 처리 패킷 전송
        WorldToClient_EnterRoom enterRoom;
        enterRoom.set_id(payload.id());
        enterRoom.set_success(true);
        PacketUtils::Send(owner, enterRoom);

        return true;
    }

    Bool ToWorld_PacketHandler::Handle_ClientToWorld_Chat(const SharedPtr<core::Session>& owner, const ClientToWorld_Chat& payload)
    {
        // 룸의 모든 플레이어에게 메시지 전송
        WorldToClient_Chat chat;
        chat.set_id(payload.id());
        chat.set_message(payload.message());
        gRoom->Broadcast(PacketUtils::MakeSendBuffer(chat, PacketId::WorldToClient_Chat), payload.id());

        return true;
    }
} // namespace world
