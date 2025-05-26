/*    GameContent/Common/Player.h    */

#pragma once

#include "ServerCore/Network/Session.h"

class Player
    : public JobSerializer
{
public:
    Player(SharedPtr<Session> session);
    Player(SharedPtr<Session> session, Int64 playerId);

    void                    SendAsync(SharedPtr<SendBuffer> buffer);
    void                    StartSendLoop(SharedPtr<SendBuffer> buffer, Int64 loopMs);
    Int64                   GetId() const { return mId; }

private:
    SharedPtr<Session>      mSession;
    Int64                   mId;

    static Atomic<Int64>    sNextId;
};

class PlayerManager
{
public:
    static PlayerManager&   GetInstance()
    {
        static PlayerManager sInstance;
        return sInstance;
    }

    void                    AddPlayer(SharedPtr<Player> player);
    void                    RemovePlayer(Int64 id);
    SharedPtr<Player>       FindPlayer(Int64 id);

private:
    PlayerManager() = default;

private:
    RW_LOCK;
    HashMap<Int64, SharedPtr<Player>>   mPlayers;
};
