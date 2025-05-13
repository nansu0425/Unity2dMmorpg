/*    GameContent/Chat/Room.cpp    */

#include "GameContent/Pch.h"
#include "GameContent/Chat/Room.h"
#include "GameContent/Common/Player.h"
#include "ServerEngine/Network/Session.h"

void Room::Enter(SharedPtr<Player> player)
{
    mPlayers.insert({player->GetId(), player});

    gLogger->Debug(TEXT_8("Player[{}]: Entered room"), player->GetId());
}

void Room::Leave(Int64 playerId)
{
    auto it = mPlayers.find(playerId);
    if (it != mPlayers.end())
    {
        mPlayers.erase(it);
    }

    gLogger->Debug(TEXT_8("Player[{}]: Left room"), playerId);
}

void Room::Broadcast(SharedPtr<SendBuffer> buffer)
{
    for (auto& [id, player] : mPlayers)
    {
        player->SendAsync(buffer);
    }
}

void Room::StartSendLoop(Int64 playerId, SharedPtr<SendBuffer> sendBuf, Int64 loopTick)
{
    auto it = mPlayers.find(playerId);
    if (it != mPlayers.end())
    {
        // 플레이어에게 전송
        auto player = it->second;
        player->SendAsync(sendBuf);

        // 다음 루프 예약
        MakeJob([self = shared_from_this(), playerId, sendBuf, loopTick]
                {
                    std::static_pointer_cast<Room>(self)->StartSendLoop(playerId, sendBuf, loopTick);
                }, loopTick);
    }
}
