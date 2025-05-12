/*    DummyClient/Network/Session.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Network/Session.h"
#include "DummyClient/Network/PacketHandler.h"
#include "GameContent/Chat/Room.h"

void ServerSession::OnConnected()
{
    gLogger->Info(TEXT_8("Session[{}]: Connected to server"), GetId());
    // 방에 입장
    // gRoom->MakeJob(&Room::Enter, GetSession());

    static Atomic<Int64> sPlayerId = 1;

    // 방 입장 요청 전송
    EnterRoomRequest payload;
    payload.set_id(sPlayerId.fetch_add(1));
    payload.set_password(TEXT_8("1234"));
    SendAsync(PacketUtils::MakeSendBuffer(payload, PacketId::EnterRoomRequest));
}

void ServerSession::OnDisconnected(String8 cause)
{
    gLogger->Warn(TEXT_8("Session[{}]: Disconnected from server: {}"), GetId(), cause);
    // 방에서 퇴장
    gRoom->MakeJob(&Room::Leave, GetPlayer());
    mPlayer.reset();
}

Int64 ServerSession::OnReceived(const Byte* buffer, Int64 numBytes)
{
    /*gLogger->Debug(TEXT_8("Session[{}]: Received {} bytes"), GetId(), numBytes);*/

    return PacketUtils::ProcessPackets(ServerPacketHandlerMap::GetInstance(), GetSession(), buffer, numBytes);
}

void ServerSession::OnSent(Int64 numBytes)
{}
