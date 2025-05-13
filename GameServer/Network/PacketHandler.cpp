/*    GameServer/Network/Packet.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/PacketHandler.h"
#include "GameServer/Network/Session.h"
#include "GameContent/Chat/Room.h"
#include "GameContent/Common/Player.h"

C2S_PacketHandlerMap& C2S_PacketHandlerMap::GetInstance()
{
    static C2S_PacketHandlerMap sInstance;
    return sInstance;
}

void C2S_PacketHandlerMap::RegisterAllHandlers()
{
    RegisterHandler(
        [this](SharedPtr<Session> session, const Byte* buffer, Int64 numBytes)
        {
            return HandlePayload<C2S_EnterRoom>(C2S_PacketHandlerMap::Handle_C2S_EnterRoom, std::move(session), buffer, numBytes);
        },
        PacketId::C2S_EnterRoom);

    RegisterHandler(
        [this](SharedPtr<Session> session, const Byte* buffer, Int64 numBytes)
        {
            return HandlePayload<C2S_Chat>(C2S_PacketHandlerMap::Handle_C2S_Chat, std::move(session), buffer, numBytes);
        },
        PacketId::C2S_Chat);
}

C2S_PacketHandlerMap::C2S_PacketHandlerMap()
{
    RegisterAllHandlers();
}

Bool C2S_PacketHandlerMap::Handle_C2S_EnterRoom(SharedPtr<Session> session, const C2S_EnterRoom& payload)
{
    gRoom->MakeJob([session, id = payload.id()]
                   {
                       // 플레이어 생성 후 방 입장
                       auto player = std::make_shared<Player>(session, id);
                       std::static_pointer_cast<ClientSession>(session)->SetPlayerId(id);
                       gRoom->Enter(std::move(player));

                       // 방 입장 응답 전송
                       S2C_EnterRoom payload;
                       payload.set_id(id);
                       payload.set_success(true);
                       session->SendAsync(PacketUtils::MakePacketBuffer(payload, PacketId::S2C_EnterRoom));
                   });


    return true;
}

Bool C2S_PacketHandlerMap::Handle_C2S_Chat(SharedPtr<Session> session, const C2S_Chat& payload)
{
    gLogger->Info(TEXT_8("Player[{}]: Chat message: {}"), payload.id(), payload.message());

    gRoom->MakeJob([session, id = payload.id(), msg = payload.message()]()
                   {
                       // 룸의 모든 플레이어에게 메시지 전송
                       S2C_Chat payload;
                       payload.set_id(id);
                       payload.set_message(msg);
                       gRoom->Broadcast(PacketUtils::MakePacketBuffer(payload, PacketId::S2C_Chat));
                   });

    return true;
}

SharedPtr<Room> gRoom = std::make_shared<Room>();
