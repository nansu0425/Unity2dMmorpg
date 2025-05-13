/*    DummyClient/Network/Session.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Network/Session.h"
#include "DummyClient/Network/PacketHandler.h"
#include "GameContent/Chat/Room.h"

void ServerSession::OnConnected()
{
    gLogger->Info(TEXT_8("Session[{}]: Connected to server"), GetId());

    static Atomic<Int64> sNextPlayerId = 1;

    // 방 입장 요청 전송
    C2S_EnterRoom payload;
    payload.set_id(sNextPlayerId.fetch_add(1));
    payload.set_password(TEXT_8("1234"));
    SendAsync(PacketUtils::MakePacketBuffer(payload, PacketId::C2S_EnterRoom));
}

void ServerSession::OnDisconnected(String8 cause)
{
    gLogger->Warn(TEXT_8("Session[{}]: Disconnected from server: {}"), GetId(), cause);

    // 방에서 퇴장
    gRoom->PushJob(&Room::Leave, GetPlayerId());
    SetPlayerId(0);
}

Int64 ServerSession::OnReceived(const Byte* buffer, Int64 numBytes)
{
    return PacketUtils::ProcessPackets(S2C_PacketHandlerMap::GetInstance(), GetSession(), buffer, numBytes);
}

void ServerSession::OnSent(Int64 numBytes)
{}

SharedPtr<Room> gRoom = std::make_shared<Room>();
