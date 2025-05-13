/*    DummyClient/Network/PacketHandler.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Network/PacketHandler.h"
#include "GameContent/Chat/Room.h"
#include "GameContent/Common/Player.h"
#include "DummyClient/Network/Session.h"

ServerPacketHandlerMap& ServerPacketHandlerMap::GetInstance()
{
    static ServerPacketHandlerMap sInstance;
    return sInstance;
}

void ServerPacketHandlerMap::RegisterAllHandlers()
{
    RegisterHandler(
        [this](SharedPtr<Session> session, const Byte* buffer, Int64 numBytes)
        {
            return HandlePayload<EnterRoomResponse>(ServerPacketHandlerMap::Handle_EnterRoomResponse, std::move(session), buffer, numBytes);
        },
        PacketId::EnterRoomResponse);

    RegisterHandler(
        [this](SharedPtr<Session> session, const Byte* buffer, Int64 numBytes)
        {
            return HandlePayload<ChatBroadcast>(ServerPacketHandlerMap::Handle_ChatBroadcast, std::move(session), buffer, numBytes);
        },
        PacketId::ChatBroadcast);
}

ServerPacketHandlerMap::ServerPacketHandlerMap()
{
    RegisterAllHandlers();
}

Bool ServerPacketHandlerMap::Handle_EnterRoomResponse(SharedPtr<Session> session, EnterRoomResponse payload)
{
    if (!payload.success())
    {
        gLogger->Error(TEXT_8("Session[{}]: Failed to enter room"), session->GetId());
        return false;
    }

    gRoom->MakeJob([session, id = payload.id()]
                   {
                       // 플레이어 생성 후 방 입장
                       auto player = std::make_shared<Player>(session, id);
                       std::static_pointer_cast<ServerSession>(session)->SetPlayerId(id);
                       gRoom->Enter(std::move(player));

                       // 1초마다 채팅 메시지 전송
                       gRoom->MakeJob([id]
                                      {
                                          ChatNotify payload;
                                          payload.set_id(id);
                                          payload.set_message(TEXT_8("Hello, world!"));
                                          gRoom->StartSendLoop(id, PacketUtils::MakeSendBuffer(payload, PacketId::ChatNotify), 1000);
                                      });
                   });

    return true;
}

Bool ServerPacketHandlerMap::Handle_ChatBroadcast(SharedPtr<Session> session, ChatBroadcast payload)
{
    gLogger->Info(TEXT_8("Player[{}]: Chat message: {}"), payload.id(), payload.message());

    return true;
}

SharedPtr<Room> gRoom = std::make_shared<Room>();
