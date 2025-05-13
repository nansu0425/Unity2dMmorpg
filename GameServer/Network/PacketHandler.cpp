/*    GameServer/Network/Packet.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/PacketHandler.h"
#include "GameServer/Network/Session.h"
#include "GameContent/Chat/Room.h"
#include "GameContent/Common/Player.h"

ClientPacketHandlerMap& ClientPacketHandlerMap::GetInstance()
{
    static ClientPacketHandlerMap instance;
    return instance;
}

void ClientPacketHandlerMap::RegisterAllHandlers()
{
    RegisterHandler(
        [this](SharedPtr<Session> session, const Byte* buffer, Int64 numBytes)
        {
            return HandlePayload<EnterRoomRequest>(ClientPacketHandlerMap::Handle_EnterRoomRequest, std::move(session), buffer, numBytes);
        },
        PacketId::EnterRoomRequest);

    RegisterHandler(
        [this](SharedPtr<Session> session, const Byte* buffer, Int64 numBytes)
        {
            return HandlePayload<ChatNotify>(ClientPacketHandlerMap::Handle_ChatNotify, std::move(session), buffer, numBytes);
        },
        PacketId::ChatNotify);
}

ClientPacketHandlerMap::ClientPacketHandlerMap()
{
    RegisterAllHandlers();
}

Bool ClientPacketHandlerMap::Handle_EnterRoomRequest(SharedPtr<Session> session, const EnterRoomRequest& payload)
{
    gRoom->MakeJob([session, id = payload.id()]
                   {
                       // 플레이어 생성 후 방 입장
                       auto player = std::make_shared<Player>(session, id);
                       std::static_pointer_cast<ClientSession>(session)->SetPlayerId(id);
                       gRoom->Enter(std::move(player));

                       // 방 입장 응답 전송
                       EnterRoomResponse payload;
                       payload.set_id(id);
                       payload.set_success(true);
                       session->SendAsync(PacketUtils::MakeSendBuffer(payload, PacketId::EnterRoomResponse));
                   });


    return true;
}

Bool ClientPacketHandlerMap::Handle_ChatNotify(SharedPtr<Session> session, ChatNotify payload)
{
    gLogger->Info(TEXT_8("Player[{}]: Chat message: {}"), payload.id(), payload.message());

    gRoom->MakeJob([session, id = payload.id(), msg = payload.message()]()
                   {
                       // 룸의 모든 플레이어에게 메시지 전송
                       ChatBroadcast broadcast;
                       broadcast.set_id(id);
                       broadcast.set_message(msg);
                       gRoom->Broadcast(PacketUtils::MakeSendBuffer(broadcast, PacketId::ChatBroadcast));
                   });

    return true;
}

SharedPtr<Room> gRoom = std::make_shared<Room>();
