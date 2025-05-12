/*    GameContent/Common/Player.h    */

#pragma once

#include "ServerEngine/Network/Session.h"

class Player
    : std::enable_shared_from_this<Player>
{
public:
    Player(SharedPtr<Session> session, Int64 id);

    void    SendAsync(SharedPtr<SendBuffer> buffer);
    Int64   GetId() const { return mId; }

private:
    SharedPtr<Session>  mSession;
    Int64               mId;
};
