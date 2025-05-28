 /*    WorldServer/Network/Session.cpp    */

#include "WorldServer/Pch.h"
#include "WorldServer/Network/Session.h"
#include "WorldServer/Network/Packet.h"
#include "Protocol/Packet/Util.h"
#include "GameLogic/Chat/Room.h"
#include "GameLogic/Common/Player.h"

using namespace core;
using namespace game;
using namespace proto;

namespace world
{
    ClientSession::~ClientSession()
    {
        gLogger->Info(TEXT_8("Session[{}]: Destroyed"), GetId());
    }

    void ClientSession::OnConnected()
    {
        gLogger->Info(TEXT_8("Session[{}]: Connected to client"), GetId());
    }

    void ClientSession::OnDisconnected(String8 cause)
    {
        gLogger->Warn(TEXT_8("Session[{}]: Disconnected from client: {}"), GetId(), cause);

        // 방에서 퇴장
        gRoom->Leave(GetPlayerId());

        // 플레이어 매니저에서 제거
        PlayerManager::GetInstance().RemovePlayer(GetPlayerId());
        SetPlayerId(0);
    }

    Int64 ClientSession::OnReceived(const Byte* buffer, Int64 numBytes)
    {
        return Client2World_PacketDispatcher::GetInstance().DispatchReceivedPackets(GetSession(), buffer, numBytes);
    }

    void ClientSession::OnSent(Int64 numBytes)
    {}

    SharedPtr<Room>     gRoom = std::make_shared<Room>();
} // namespace world
