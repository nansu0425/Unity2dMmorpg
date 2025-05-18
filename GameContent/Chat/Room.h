/*    GameContent/Chat/Room.h    */

#pragma once

class Player;
class Session;

class Room
    : public JobSerializer
{
public:
    void    Enter(SharedPtr<Player> player);
    void    Leave(Int64 playerId);
    void    Broadcast(SharedPtr<SendBuffer> buffer, Int64 playerId = 0);
    void    StartBroadcastLoop(SharedPtr<SendBuffer> buffer, Int64 loopMs);

private:
    RW_LOCK;
    HashMap<Int64, SharedPtr<Player>>   mPlayers;
};
