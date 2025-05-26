/*    GameContent/Common/Player.h    */

#pragma once

#include "ServerCore/Network/Session.h"

namespace game
{
    class Player
        : public core::JobSerializer
    {
    public:
        Player(SharedPtr<core::Session> session);
        Player(SharedPtr<core::Session> session, Int64 playerId);

        void                    SendAsync(SharedPtr<core::SendBuffer> buffer);
        void                    StartSendLoop(SharedPtr<core::SendBuffer> buffer, Int64 loopMs);
        Int64                   GetId() const { return mId; }

    private:
        SharedPtr<core::Session>    mSession;
        Int64                       mId;

        static Atomic<Int64>        sNextId;
    };

    class PlayerManager
    {
    public:
        static PlayerManager& GetInstance()
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
} // namespace game
