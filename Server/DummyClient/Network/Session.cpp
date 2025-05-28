/*    DummyClient/Network/Session.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Network/Session.h"
#include "DummyClient/Network/PacketHandler.h"
#include "Protocol/Packet/Util.h"
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
        Client2World_EnterRoom payload;
        payload.set_id(GetPlayerId());
        payload.set_password(TEXT_8("1234"));
        SendAsync(util::MakePacketSendBuffer(payload, PacketId::Client2World_EnterRoom));
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
        return World2Client_PacketDispatcher::GetInstance().DispatchReceivedPackets(GetSession(), buffer, numBytes);
    }

    void ServerSession::OnSent(Int64 numBytes)
    {}

    SharedPtr<Room>     gRoom = std::make_shared<Room>();
} // namespace dummy
