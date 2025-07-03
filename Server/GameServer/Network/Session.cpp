 /*    GameServer/Network/Session.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/Session.h"
#include "GameServer/Packet/Handler.h"
#include "GameLogic/Chat/Room.h"
#include "GameLogic/Entity/Player.h"
#include "GameLogic/Core/Loop.h"

namespace game
{
    ClientSession::~ClientSession()
    {
        core::gLogger->Info(TEXT_8("Session[{}]: Destroyed"), GetId());
    }

    void ClientSession::OnConnected()
    {
        core::gLogger->Info(TEXT_8("Session[{}]: Connected to client"), GetId());
    }

    void ClientSession::OnDisconnected(String8 cause)
    {
        core::gLogger->Warn(TEXT_8("Session[{}]: Disconnected from client: {}"), GetId(), cause);

        // 방에서 퇴장
        gRoom->Leave(GetPlayerId());

        // 플레이어 매니저에서 제거
        PlayerManager::GetInstance().RemovePlayer(GetPlayerId());
        SetPlayerId(0);
    }

    Int64 ClientSession::OnReceived(const Byte* buffer, Int64 numBytes)
    {
        return game::Loop::GetInstance().PushPackets(GetSession(), buffer, numBytes);
    }

    void ClientSession::OnSent(Int64 numBytes)
    {}

    SharedPtr<Room>     gRoom = std::make_shared<Room>();
} // namespace world
