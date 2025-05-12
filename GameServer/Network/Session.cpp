 /*    GameServer/Network/Session.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/Session.h"
#include "GameServer/Network/PacketHandler.h"
#include "GameContent/Chat/Room.h"

void ClientSession::OnConnected()
{
    gLogger->Info(TEXT_8("Session[{}]: Connected to client"), GetId());
    // 방에 입장
    // gRoom->MakeJob(&Room::Enter, GetSession());
}

void ClientSession::OnDisconnected(String8 cause)
{
    gLogger->Warn(TEXT_8("Session[{}]: Disconnected from client: {}"), GetId(), cause);
    // 방에서 퇴장
    gRoom->MakeJob(&Room::Leave, GetPlayer());
    mPlayer.reset();
}

Int64 ClientSession::OnReceived(const Byte* buffer, Int64 numBytes)
{
    //gLogger->Debug(TEXT_8("Session[{}]: Received {} bytes"), GetId(), numBytes);

    //// 수신 버퍼 데이터를 룸의 모든 플레이어에게 전송
    //SharedPtr<SendBuffer> sendBuf = gSendChunkPool->Alloc(1024);
    //BufferWriter writer(sendBuf->GetBuffer(), sendBuf->GetAllocSize());
    //writer.Write(buffer, numBytes);
    //sendBuf->OnWritten(numBytes);
    //gRoom->MakeJob(&Room::Broadcast, std::move(sendBuf));

    return PacketUtils::ProcessPackets(ClientPacketHandlerMap::GetInstance(), GetSession(), buffer, numBytes);
}

void ClientSession::OnSent(Int64 numBytes)
{}
