/*    GameServer/Chat/Room.h    */

#pragma once

namespace game
{
    class Player;

    class Room
        : public core::JobSerializer
    {
    public:
        void        Enter(SharedPtr<Player> player);
        void        Leave(Int64 playerId);
        void        Broadcast(SharedPtr<core::SendBuffer> buffer, Int64 playerId = 0);
        void        StartBroadcastLoop(SharedPtr<core::SendBuffer> buffer, Int64 loopMs);

    private:
        RW_LOCK;
        HashMap<Int64, SharedPtr<Player>>   mPlayers;
    };
} // namespace game
