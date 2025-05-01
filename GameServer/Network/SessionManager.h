/*    GameServer/Network/SessionManager.h    */

#pragma once

class GameSession;
class NetMessage;

class GameSessionManager
{
public:
    GameSessionManager()    = default;
    ~GameSessionManager()   = default;

    void    AddSession(SharedPtr<GameSession> session);
    void    RemoveSession(SharedPtr<GameSession> session);
    void    Broadcast(SharedPtr<NetMessage> message);

private:
    RW_LOCK;
    HashSet<SharedPtr<GameSession>>     mSessions;
};

extern GameSessionManager               gSessionManager;
