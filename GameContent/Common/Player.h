/*    GameContent/Common/Player.h    */

#pragma once

#include "ServerEngine/Network/Session.h"

class Player
    : std::enable_shared_from_this<Player>
{
public:
    Player(SharedPtr<Session> session);

    void    Send(SharedPtr<SendBuffer> buffer);
    Int64   GetId() const { return mSession->GetId(); }

private:
    SharedPtr<Session>  mSession;
};
