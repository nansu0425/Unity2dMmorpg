/*    GameServer/Content/Room.h    */

#pragma once

struct Player;

class Room
    : public JobSerializer
{
public:
    void    Enter(SharedPtr<Player> player);
    void    Leave(SharedPtr<Player> player);
    void    Broadcast(SharedPtr<SendMessageBuilder> message);

private:
    TreeMap<Int64, SharedPtr<Player>>   mPlayers;
};

extern SharedPtr<Room> gRoom;
