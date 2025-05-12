/*    GameContent/Chat/Room.cpp    */

#include "GameContent/Pch.h"
#include "GameContent/Chat/Room.h"
#include "GameContent/Common/Player.h"
#include "ServerEngine/Network/Session.h"

void Room::Enter(SharedPtr<Session> session)
{
    auto player = std::make_shared<Player>(session);
    mPlayerMap.insert({player->GetId(), player});
}

void Room::Leave(SharedPtr<Session> session)
{
    auto it = mPlayerMap.find(session->GetId());
    if (it != mPlayerMap.end())
    {
        mPlayerMap.erase(it);
    }
}

void Room::Broadcast(SharedPtr<SendBuffer> buffer)
{
    for (auto& [id, player] : mPlayerMap)
    {
        player->SendAsync(buffer);
    }
}
