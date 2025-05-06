/*    GameServer/Content/Room.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Content/Room.h"
#include "GameServer/Content/Player.h"
#include "GameServer/Network/Session.h"

Room gRoom;

void Room::Enter(SharedPtr<Player> player)
{
    WRITE_GUARD;
    mPlayers.emplace(player->id, player);
}

void Room::Leave(SharedPtr<Player> player)
{
    WRITE_GUARD;
    auto it = mPlayers.find(player->id);
    if (it != mPlayers.end())
    {
        mPlayers.erase(it);
    }
}

void Room::Broadcast(SharedPtr<SendMessageBuilder> message)
{
    WRITE_GUARD;
    for (auto& [id, player] : mPlayers)
    {
        player->owner->Send(message);
    }
}
