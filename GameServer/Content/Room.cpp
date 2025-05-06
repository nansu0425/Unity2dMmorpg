/*    GameServer/Content/Room.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Content/Room.h"
#include "GameServer/Content/Player.h"
#include "GameServer/Network/Session.h"

SharedPtr<Room> gRoom = std::make_shared<Room>();

void Room::FlushJobs()
{
    while (true)
    {
        SharedPtr<Job> job = mJobs.Pop();
        if (job == nullptr)
        {
            break;
        }
        job->Execute();
    }
}

void Room::Enter(SharedPtr<Player> player)
{
    mPlayers.emplace(player->id, player);
}

void Room::Leave(SharedPtr<Player> player)
{
    auto it = mPlayers.find(player->id);
    if (it != mPlayers.end())
    {
        mPlayers.erase(it);
    }
}

void Room::Broadcast(SharedPtr<SendMessageBuilder> message)
{
    for (auto& [id, player] : mPlayers)
    {
        player->owner->Send(message);
    }
}
