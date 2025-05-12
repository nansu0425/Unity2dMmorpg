/*    GameServer/Network/Session.h    */

#pragma once

#include "ServerEngine/Network/Session.h"
#include "GameContent/Common/Player.h"

class ClientSession
    : public Session
{
public:
    void                SetPlayer(WeakPtr<Player> player) { mPlayer = std::move(player); }
    SharedPtr<Player>   GetPlayer() const { return mPlayer.lock(); }

protected:
    virtual void        OnConnected() override;
    virtual void        OnDisconnected(String8 cause) override;
    virtual Int64       OnReceived(const Byte* buffer, Int64 numBytes) override;
    virtual void        OnSent(Int64 numBytes) override;

private:
    WeakPtr<Player>     mPlayer;
};
