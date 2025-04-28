/*    GameServer/Network/Session.h    */

#pragma once

#include "ServerEngine/Network/Session.h"

class GameSession
    : public PacketSession
{
protected:
    virtual void    OnConnected() override;
    virtual void    OnDisconnected(String16 cause) override;
    virtual void    OnPacketReceived(Byte* packet, Int64 size) override;
    virtual void    OnSent(Int64 numBytes) override;
};
