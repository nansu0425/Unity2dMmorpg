/*    GameContent/Common/Player.h    */

#pragma once

#include "ServerEngine/Network/Session.h"

class Player
    : public JobSerializer
{
public:
    Player(SharedPtr<Session> session, Int64 id);

    void                    SendAsync(SharedPtr<SendBuffer> buffer);
    void                    StartSendLoop(SharedPtr<SendBuffer> sendBuf, Int64 loopTick);
    Int64                   GetId() const { return mId; }

private:
    SharedPtr<Session>      mSession;
    Int64                   mId;
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
    SharedPtr<Player>       GetPlayer(Int64 id);

private:
    PlayerManager() = default;

private:
    RW_LOCK;
    HashMap<Int64, SharedPtr<Player>>   mPlayers;
};
