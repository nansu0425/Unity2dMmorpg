/*    GameContent/Chat/Room.h    */

#pragma once

class Player;
class Session;

class Room
    : public JobSerializer
{
public:
    void Enter(SharedPtr<Session> session);
    void Leave(SharedPtr<Session> session);
    void Broadcast(SharedPtr<SendBuffer> buffer);

private:
    HashMap<Int64, SharedPtr<Player>>  mPlayerMap;
};
