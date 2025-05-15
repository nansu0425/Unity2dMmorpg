/*    GameContent/Common/Player.cpp    */

#include "GameContent/Pch.h"
#include "GameContent/Common/Player.h"

Player::Player(SharedPtr<Session> session, Int64 id)
    : mSession(std::move(session))
    , mId(id)
{}

void Player::SendAsync(SharedPtr<SendBuffer> buffer)
{
    mSession->SendAsync(std::move(buffer));
}

void Player::StartSendLoop(SharedPtr<SendBuffer> sendBuf, Int64 loopMs)
{
    // 매니저에 없으면 반복 종료
    if (PlayerManager::GetInstance().GetPlayer(mId) == nullptr)
    {
        return;
    }

    // 송신 버퍼 전송
    SendAsync(sendBuf);
    // 다음 루프 예약
    ScheduleJob(loopMs, &Player::StartSendLoop, std::move(sendBuf), loopMs);
}

void PlayerManager::AddPlayer(SharedPtr<Player> player)
{
    WRITE_GUARD;

    mPlayers.insert({player->GetId(), player});
    gLogger->Info(TEXT_8("Player[{}]: Added to manager"), player->GetId());
}

void PlayerManager::RemovePlayer(Int64 id)
{
    WRITE_GUARD;

    auto it = mPlayers.find(id);
    if (it != mPlayers.end())
    {
        mPlayers.erase(it);
        gLogger->Info(TEXT_8("Player[{}]: Removed from manager"), id);
    }
}

SharedPtr<Player> PlayerManager::GetPlayer(Int64 id)
{
    READ_GUARD;

    auto it = mPlayers.find(id);
    if (it != mPlayers.end())
    {
        return it->second;
    }

    return nullptr;
}
