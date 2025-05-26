/*    DummyClient/Network/Session.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Network/Session.h"
#include "DummyClient/Network/PacketHandler.h"
#include "GameContent/Chat/Room.h"
#include "GameContent/Common/Player.h"

ServerSession::~ServerSession()
{
    gLogger->Info(TEXT_8("Session[{}]: Destroyed"), GetId());
}

void ServerSession::OnConnected()
{
    gLogger->Info(TEXT_8("Session[{}]: Connected to server"), GetId());

    // 플레이어 생성 및 매니저에 추가
    auto player = std::make_shared<Player>(GetSession());
    SetPlayerId(player->GetId());
    PlayerManager::GetInstance().AddPlayer(std::move(player));

    // 방 입장 요청 전송
    C2S_EnterRoom payload;
    payload.set_id(GetPlayerId());
    payload.set_password(TEXT_8("1234"));
    SendAsync(PacketUtils::MakePacketBuffer(payload, PacketId::C2S_EnterRoom));
}

void ServerSession::OnDisconnected(String8 cause)
{
    gLogger->Warn(TEXT_8("Session[{}]: Disconnected from server: {}"), GetId(), cause);

    // 방에서 퇴장
    gRoom->Leave(GetPlayerId());

    // 플레이어 매니저에서 제거
    PlayerManager::GetInstance().RemovePlayer(GetPlayerId());
    SetPlayerId(0);
}

Int64 ServerSession::OnReceived(const Byte* buffer, Int64 numBytes)
{
    return PacketUtils::ProcessPackets(S2C_PacketHandlerMap::GetInstance(), GetSession(), buffer, numBytes);
}

void ServerSession::OnSent(Int64 numBytes)
{}

SharedPtr<Room>     gRoom = std::make_shared<Room>();
