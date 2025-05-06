/*    GameServer/Content/Room.h    */

#pragma once

#include "ServerEngine/Job/Serializer.h"

struct Player;

class Room
    : public JobSerializer
{
public:     // JobSerializer 인터페이스 구현
    virtual void FlushJobs() override;

public:
    void Enter(SharedPtr<Player> player);
    void Leave(SharedPtr<Player> player);
    void Broadcast(SharedPtr<SendMessageBuilder> message);

private:
    TreeMap<Int64, SharedPtr<Player>>   mPlayers;
};

extern SharedPtr<Room> gRoom;
