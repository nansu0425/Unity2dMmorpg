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
