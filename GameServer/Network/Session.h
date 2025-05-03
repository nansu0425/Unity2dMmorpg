/*    GameServer/Network/Session.h    */

#pragma once

#include "ServerEngine/Network/Session.h"

class GameSession
    : public Session
{
public:
    void            SetLogined() { mIsLogined.store(true); }
    Bool            IsLogined() const { return mIsLogined.load(); }

protected:
    virtual void    OnConnected() override;
    virtual void    OnDisconnected(String8 cause) override;
    virtual void    OnReceived(ReceiveMessage message) override;
    virtual void    OnSent(Int64 numBytes) override;

private:
    Atomic<Bool>    mIsLogined = false;
};
