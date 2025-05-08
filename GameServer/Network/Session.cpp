 /*    GameServer/Network/Session.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/Session.h"
#include "GameServer/Network/Message.h"

void ClientSession::OnConnected()
{
    gLogger->Info(TEXT_8("Connected to client"));

    // 세션 매니저에 세션 추가
    gClientManager.AddSession(std::static_pointer_cast<ClientSession>(shared_from_this()));
}

void ClientSession::OnDisconnected(String8 cause)
{
    gLogger->Warn(TEXT_8("Disconnected from client: {}"), cause);

    // 세션 매니저에서 세션 제거
    gClientManager.RemoveSession(std::static_pointer_cast<ClientSession>(shared_from_this()));
}

void ClientSession::OnReceived(ReceiveMessage message)
{
    // 메시지 처리
    gMessageHandlerManager.HandleMessage(GetSharedPtr(), message);
}

void ClientSession::OnSent(Int64 numBytes)
{}

void ClientSessionManager::AddSession(SharedPtr<ClientSession> client)
{
    WRITE_GUARD;
    mClients.insert(client);
}

void ClientSessionManager::RemoveSession(SharedPtr<ClientSession> client)
{
    WRITE_GUARD;
    auto it = mClients.find(client);
    if (it != mClients.end())
    {
        mClients.erase(it);
    }
}

void ClientSessionManager::Broadcast(SharedPtr<SendMessageBuilder> message)
{
    WRITE_GUARD;
    for (auto& client : mClients)
    {
        if (client->IsLogined())
        {
            client->Send(message);
        }
    }
}

ClientSessionManager gClientManager;
