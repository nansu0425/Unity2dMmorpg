/*    DummyClient/Network/Session.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Network/Session.h"

void ServerSession::OnConnected()
{
    gLogger->Info(TEXT_8("Session[{}]: Connected to server"), GetId());
    // 서버 매니저에 세션 추가
    gServerManager->MakeJob(&ServerSessionManager::AddSession, std::static_pointer_cast<ServerSession>(shared_from_this()));
}

void ServerSession::OnDisconnected(String8 cause)
{
    gLogger->Warn(TEXT_8("Session[{}]: Disconnected from server: {}"), GetId(), cause);
    // 서버 매니저에서 세션 제거
    gServerManager->MakeJob(&ServerSessionManager::RemoveSession, std::static_pointer_cast<ServerSession>(shared_from_this()));
}

Int64 ServerSession::OnReceived(const Byte* buffer, Int64 numBytes)
{
    gLogger->Debug(TEXT_8("Session[{}]: Received {} bytes"), GetId(), numBytes);
    return numBytes;
}

void ServerSession::OnSent(Int64 numBytes)
{}

void ServerSessionManager::AddSession(SharedPtr<ServerSession> server)
{
    mServers.insert(server);
}

void ServerSessionManager::RemoveSession(SharedPtr<ServerSession> server)
{
    mServers.erase(server);
}

void ServerSessionManager::Broadcast(SharedPtr<SendBuffer> buffer)
{
    for (auto& server : mServers)
    {
        server->Send(buffer);
    }
}

SharedPtr<ServerSessionManager>     gServerManager = std::make_shared<ServerSessionManager>();
