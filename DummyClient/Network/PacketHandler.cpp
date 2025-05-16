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

    gRoom->PushJob([owner, id = payload.id()]
                   {
                       // 플레이어 찾기
                       auto player = PlayerManager::GetInstance().GetPlayer(id);
                       if (player == nullptr)
                       {
                           gLogger->Error(TEXT_8("Session[{}]: Player[{}] not found"), owner->GetId(), id);
                           return;
                       }

                       // 방 입장
                       gRoom->Enter(player);

                       // 100ms마다 채팅 메시지 전송
                       C2S_Chat payload;
                       payload.set_id(id);
                       payload.set_message(TEXT_8("Hello, world!"));
                       player->StartSendLoop(PacketUtils::MakePacketBuffer(payload, PacketId::C2S_Chat), 1000);
                   });

    return true;
}

Bool S2C_PacketHandlerMap::Handle_S2C_Chat(SharedPtr<Session> owner, const S2C_Chat& payload)
{
    return true;
}
