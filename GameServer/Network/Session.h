/*    GameServer/Network/Session.h    */

#pragma once

#include "ServerEngine/Network/Session.h"

class GameSession
    : public Session
{
protected:
    virtual void    OnConnected() override;
    virtual void    OnDisconnected(String16 cause) override;
    virtual Int64   OnReceived(Byte* buffer, Int64 numBytes) override;
    virtual void    OnSent(Int64 numBytes) override;
};
