/*    GameServer/Network/Session.h    */

#pragma once

#include "ServerEngine/Network/Session.h"

class GameSession
    : public Session
{
protected:
    virtual void    OnConnected() override;
    virtual void    OnDisconnected(String16 cause) override;
    virtual void    OnReceived(MessageHeader* header) override;
    virtual void    OnSent(Int64 numBytes) override;
};
