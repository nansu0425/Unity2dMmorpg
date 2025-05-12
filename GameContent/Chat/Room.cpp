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

void Room::Leave(SharedPtr<Player> player)
{
    auto it = mPlayers.find(player->GetId());
    if (it != mPlayers.end())
    {
        mPlayers.erase(it);
    }

    gLogger->Debug(TEXT_8("Player[{}]: Left room"), player->GetId());
}

void Room::Broadcast(SharedPtr<SendBuffer> buffer)
{
    for (auto& [id, player] : mPlayers)
    {
        player->SendAsync(buffer);
    }
}
