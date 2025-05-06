/*    GameServer/Content/Room.h    */

#pragma once

struct Player;

class Room
{
public:
    void Enter(SharedPtr<Player> player);
    void Leave(SharedPtr<Player> player);
    void Broadcast(SharedPtr<SendMessageBuilder> message);

private:
    RW_LOCK;
    TreeMap<Int64, SharedPtr<Player>>   mPlayers;
};

extern Room gRoom;
