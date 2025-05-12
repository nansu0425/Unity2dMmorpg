/*    GameContent/Common/Player.cpp    */

#include "GameContent/Pch.h"
#include "GameContent/Common/Player.h"

Player::Player(SharedPtr<Session> session)
    : mSession(std::move(session))
{}

void Player::Send(SharedPtr<SendBuffer> buffer)
{
    mSession->Send(std::move(buffer));
}
