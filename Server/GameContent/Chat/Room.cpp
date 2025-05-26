/*    GameContent/Chat/Room.cpp    */

#include "GameContent/Pch.h"
#include "GameContent/Chat/Room.h"
#include "GameContent/Common/Player.h"
#include "ServerCore/Network/Session.h"

using namespace core;

namespace game
{
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
        Vector<SharedPtr<Player>> targets;
        {
            WRITE_GUARD;
            // 자신을 제외한 모든 플레이어 대상
            for (auto& [id, player] : mPlayers)
            {
                if (id == playerId)
                {
                    continue;
                }
                targets.push_back(player);
            }
        }

        // 메시지 전송
        for (auto& player : targets)
        {
            player->SendAsync(buffer);
        }

        gLogger->Info(TEXT_8("Player[{}]: Broadcasted message"), playerId);
    }

    void Room::StartBroadcastLoop(SharedPtr<SendBuffer> buffer, Int64 loopMs)
    {
        const Int64 nextTick = ::GetTickCount64() + loopMs;

        Vector<SharedPtr<Player>> targets;
        {
            WRITE_GUARD;
            // 모든 플레이어 대상
            for (auto& [id, player] : mPlayers)
            {
                targets.push_back(player);
            }
        }

        // 메시지 전송
        for (auto& player : targets)
        {
            player->SendAsync(buffer);
        }

        gLogger->Info(TEXT_8("Room: Broadcasted message to all players"));

        const Int64 delayMs = nextTick - ::GetTickCount64();

        // 다음 루프 예약
        ScheduleJob(delayMs, [self = shared_from_this(), buffer, loopMs]()
                    {
                        std::static_pointer_cast<Room>(self)->StartBroadcastLoop(buffer, loopMs);
                    });
    }
} // namespace game
