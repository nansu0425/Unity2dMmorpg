/*    DummyClient/Network/PacketHandler.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Network/PacketHandler.h"
#include "GameContent/Chat/Room.h"
#include "GameContent/Common/Player.h"
#include "DummyClient/Network/Session.h"

S2C_PacketHandlerMap& S2C_PacketHandlerMap::GetInstance()
{
    static S2C_PacketHandlerMap sInstance;
    return sInstance;
}

void S2C_PacketHandlerMap::RegisterAllHandlers()
{
    RegisterHandler(
        [this](SharedPtr<Session> session, const Byte* buffer, Int64 numBytes)
        {
            return HandlePayload<S2C_EnterRoom>(S2C_PacketHandlerMap::Handle_S2C_EnterRoom, std::move(session), buffer, numBytes);
        },
        PacketId::S2C_EnterRoom);

    RegisterHandler(
        [this](SharedPtr<Session> session, const Byte* buffer, Int64 numBytes)
        {
            return HandlePayload<S2C_Chat>(S2C_PacketHandlerMap::Handle_S2C_Chat, std::move(session), buffer, numBytes);
        },
        PacketId::S2C_Chat);
}

S2C_PacketHandlerMap::S2C_PacketHandlerMap()
{
    RegisterAllHandlers();
}

Bool S2C_PacketHandlerMap::Handle_S2C_EnterRoom(SharedPtr<Session> session, const S2C_EnterRoom& payload)
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
                                          C2S_Chat payload;
                                          payload.set_id(id);
                                          payload.set_message(TEXT_8("Hello, world!"));
                                          gRoom->StartSendLoop(id, PacketUtils::MakePacketBuffer(payload, PacketId::C2S_Chat), 1000);
                                      });
                   });

    return true;
}

Bool S2C_PacketHandlerMap::Handle_S2C_Chat(SharedPtr<Session> session, const S2C_Chat& payload)
{
    gLogger->Info(TEXT_8("Player[{}]: Chat message: {}"), payload.id(), payload.message());

    return true;
}

SharedPtr<Room> gRoom = std::make_shared<Room>();
