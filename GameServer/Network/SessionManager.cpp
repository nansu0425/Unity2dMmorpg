/*    GameServer/Network/SessionManager.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/SessionManager.h"
#include "GameServer/Network/Session.h"
#include "ServerEngine/Network/Message.h"

GameSessionManager gSessionManager;

void GameSessionManager::AddSession(SharedPtr<GameSession> session)
{
    WRITE_GUARD;
    mSessions.insert(session);
}

void GameSessionManager::RemoveSession(SharedPtr<GameSession> session)
{
    WRITE_GUARD;
    auto it = mSessions.find(session);
    if (it != mSessions.end())
    {
        mSessions.erase(it);
    }
}

void GameSessionManager::Broadcast(SharedPtr<SendBuffer> buffer)
{
    WRITE_GUARD;
    for (auto& session : mSessions)
    {
        session->Send(buffer);
    }
}

void GameSessionManager::Broadcast(SharedPtr<NetMessage> message)
{
    WRITE_GUARD;
    for (auto& session : mSessions)
    {
        session->Send(message);
    }
}
