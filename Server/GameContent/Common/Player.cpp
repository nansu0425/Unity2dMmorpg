/*    GameContent/Common/Player.cpp    */

#include "GameContent/Pch.h"
#include "GameContent/Common/Player.h"

Player::Player(SharedPtr<Session> session)
    : mSession(std::move(session))
    , mId(sNextId.fetch_add(1))
{}

Player::Player(SharedPtr<Session> session, Int64 playerId)
    : mSession(std::move(session))
    , mId(playerId)
{}

void Player::SendAsync(SharedPtr<SendBuffer> buffer)
{
    mSession->SendAsync(std::move(buffer));
}

void Player::StartSendLoop(SharedPtr<SendBuffer> buffer, Int64 loopMs)
{
    const Int64 nextTick = ::GetTickCount64() + loopMs;

    // 매니저에 없으면 반복 종료
    if (PlayerManager::GetInstance().FindPlayer(mId) == nullptr)
    {
        return;
    }

    // 송신 버퍼 전송
    SendAsync(buffer);

    // 지연 시간 계산
    Int64 delayMs = nextTick - ::GetTickCount64();
    if (delayMs < 0)
    {
        gLogger->Warn(TEXT_8("Player[{}]: Send loop delay is negative"), mId);
        delayMs = 0;
    }

    // 다음 루프 예약
    ScheduleJob(delayMs, &Player::StartSendLoop, std::move(buffer), loopMs);
}

void PlayerManager::AddPlayer(SharedPtr<Player> player)
{
    Bool result = true;
    {
        WRITE_GUARD;
        // 플레이어 추가
        result = (mPlayers.insert({ player->GetId(), player }).second);
    }

    if (result)
    {
        gLogger->Info(TEXT_8("Player[{}]: Added to manager"), player->GetId());
    }
    else
    {
        gLogger->Error(TEXT_8("Player[{}]: Already exists in manager"), player->GetId());
    }
}

void PlayerManager::RemovePlayer(Int64 id)
{
    Bool result = true;
    {
        WRITE_GUARD;
        // 플레이어 제거
        result = (mPlayers.erase(id) > 0);
    }

    if (result)
    {
        gLogger->Info(TEXT_8("Player[{}]: Removed from manager"), id);
    }
    else
    {
        gLogger->Error(TEXT_8("Player[{}]: Not found in manager"), id);
    }
}

SharedPtr<Player> PlayerManager::FindPlayer(Int64 id)
{
    WRITE_GUARD;

    // 플레이어 찾기
    auto it = mPlayers.find(id);
    if (it != mPlayers.end())
    {
        return it->second;
    }

    return nullptr;
}

Atomic<Int64>   Player::sNextId = 1;
