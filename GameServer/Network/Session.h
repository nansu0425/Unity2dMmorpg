/*    GameServer/Network/Session.h    */

#pragma once

#include "ServerEngine/Network/Session.h"

struct Player;

class ClientSession
    : public Session
{
public:
    void                SetLogined() { mIsLogined.store(true); }
    Bool                IsLogined() const { return mIsLogined.load(); }
    SharedPtr<Player>   GetPlayer(Int64 idx) const { return mPlayers[idx]; }
    void                AddPlayer(SharedPtr<Player> player) { mPlayers.push_back(std::move(player)); }

protected:
    virtual void        OnConnected() override;
    virtual void        OnDisconnected(String8 cause) override;
    virtual void        OnReceived(ReceiveMessage message) override;
    virtual void        OnSent(Int64 numBytes) override;

private:
    Atomic<Bool>                mIsLogined = false;
    Vector<SharedPtr<Player>>   mPlayers;
};

class ClientSessionManager
{
public:
    ClientSessionManager() = default;
    ~ClientSessionManager() = default;

    void    AddSession(SharedPtr<ClientSession> client);
    void    RemoveSession(SharedPtr<ClientSession> client);
    void    Broadcast(SharedPtr<SendMessageBuilder> message);

private:
    RW_LOCK;
    HashSet<SharedPtr<ClientSession>>     mClients;
};

extern ClientSessionManager               gClientManager;
