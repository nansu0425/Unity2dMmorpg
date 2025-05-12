/*    GameContent/Common/Player.cpp    */

#include "GameContent/Pch.h"
#include "GameContent/Common/Player.h"

Player::Player(SharedPtr<Session> session)
    : mSession(std::move(session))
{}

void Player::SendAsync(SharedPtr<SendBuffer> buffer)
{
    mSession->SendAsync(std::move(buffer));
}
