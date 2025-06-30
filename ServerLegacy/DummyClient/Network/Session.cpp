/*    DummyClient/Network/Session.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Network/Session.h"
#include "DummyClient/Packet/Handler.h"
#include "Protocol/Packet/Utils.h"
#include "GameLogic/Chat/Room.h"
#include "GameLogic/Common/Player.h"

using namespace core;
using namespace game;
using namespace proto;

namespace dummy
{
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
        ClientToWorld_EnterRoom payload;
        payload.set_id(GetPlayerId());
        payload.set_password(TEXT_8("1234"));
        PacketUtils::Send(GetSession(), payload);
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
        return ToClient_PacketHandler::GetInstance().PushPackets(GetSession(), buffer, numBytes);
    }

    void ServerSession::OnSent(Int64 numBytes)
    {}

    SharedPtr<Room>     gRoom = std::make_shared<Room>();
} // namespace dummy
