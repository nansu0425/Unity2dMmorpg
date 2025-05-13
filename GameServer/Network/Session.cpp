 /*    GameServer/Network/Session.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/Session.h"
#include "GameServer/Network/PacketHandler.h"
#include "GameContent/Chat/Room.h"

void ClientSession::OnConnected()
{
    gLogger->Info(TEXT_8("Session[{}]: Connected to client"), GetId());
}

void ClientSession::OnDisconnected(String8 cause)
{
    gLogger->Warn(TEXT_8("Session[{}]: Disconnected from client: {}"), GetId(), cause);

    // 방에서 퇴장
    gRoom->MakeJob(&Room::Leave, GetPlayerId());
    SetPlayerId(0);
}

Int64 ClientSession::OnReceived(const Byte* buffer, Int64 numBytes)
{
    return PacketUtils::ProcessPackets(C2S_PacketHandlerMap::GetInstance(), GetSession(), buffer, numBytes);
}

void ClientSession::OnSent(Int64 numBytes)
{}
