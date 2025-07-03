/*    GameServer/Common/Player.h    */

#pragma once

#include "Core/Network/Session.h"

namespace game
{
    using PlayerId = Int64;

    class Player
        : public core::JobSerializer
    {
    public:
        Player(SharedPtr<core::Session> session);
        Player(SharedPtr<core::Session> session, PlayerId id);

        void                    SendAsync(SharedPtr<core::SendBuffer> buffer);
        void                    StartSendLoop(SharedPtr<core::SendBuffer> buffer, Int64 loopMs);
        PlayerId                GetId() const { return mId; }

    private:
        SharedPtr<core::Session> mSession;
        PlayerId mId;

        static Atomic<PlayerId> sNextId;
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
        void                    RemovePlayer(PlayerId id);
        SharedPtr<Player>       FindPlayer(PlayerId id);

    private:
        PlayerManager() = default;

    private:
        RW_LOCK;
        HashMap<PlayerId, SharedPtr<Player>>   mPlayers;
    };
} // namespace game
