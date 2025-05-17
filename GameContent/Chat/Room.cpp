/*    GameContent/Chat/Room.cpp    */

#include "GameContent/Pch.h"
#include "GameContent/Chat/Room.h"
#include "GameContent/Common/Player.h"
#include "ServerEngine/Network/Session.h"

void Room::Enter(SharedPtr<Player> player)
{
    Bool result = true;
    {
        WRITE_GUARD;
        // 플레이어 추가
        result = mPlayers.insert({player->GetId(), player}).second;
    }

    if (result)
    {
        gLogger->Info(TEXT_8("Player[{}]: Entered room"), player->GetId());
    }
    else
    {
        gLogger->Error(TEXT_8("Player[{}]: Failed to enter room"), player->GetId());
    }
}

void Room::Leave(Int64 playerId)
{
    Bool result = true;
    {
        WRITE_GUARD;
        // 플레이어 제거
        result = (mPlayers.erase(playerId) > 0);
    }

    if (result)
    {
        gLogger->Info(TEXT_8("Player[{}]: Left room"), playerId);
    }
    else
    {
        gLogger->Error(TEXT_8("Player[{}]: Failed to leave room"), playerId);
    }
    
}

void Room::Broadcast(SharedPtr<SendBuffer> buffer, Int64 playerId)
{
    {
        WRITE_GUARD;
        // 자신을 제외한 모든 플레이어에게 전송
        for (auto& [id, player] : mPlayers)
        {
            if (id == playerId)
            {
                continue;
            }
            player->SendAsync(buffer);
        }
    }

    gLogger->Info(TEXT_8("Player[{}]: Broadcasted message"), playerId);
}
