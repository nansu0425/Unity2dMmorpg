 /*    GameServer/Network/Session.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/Session.h"
#include "GameServer/Network/Message.h"
#include "GameContent/Chat/Room.h"

SharedPtr<Room>     gRoom = std::make_shared<Room>();

void ClientSession::OnConnected()
{
    gLogger->Info(TEXT_8("Session[{}]: Connected to client"), GetId());
    // 세션 매니저에 세션 추가
    // gClientManager->MakeJob(&ClientSessionManager::AddSession, std::static_pointer_cast<ClientSession>(shared_from_this()));
    // 방에 입장
    gRoom->MakeJob(&Room::Enter, GetSharedPtr());
}

void ClientSession::OnDisconnected(String8 cause)
{
    gLogger->Warn(TEXT_8("Session[{}]: Disconnected from client: {}"), GetId(), cause);
    // 세션 매니저에서 세션 제거
    // gClientManager->MakeJob(&ClientSessionManager::RemoveSession, std::static_pointer_cast<ClientSession>(shared_from_this()));
    // 방에서 퇴장
    gRoom->MakeJob(&Room::Leave, GetSharedPtr());
}

Int64 ClientSession::OnReceived(const Byte* buffer, Int64 numBytes)
{
    gLogger->Debug(TEXT_8("Session[{}]: Received {} bytes"), GetId(), numBytes);

    // 수신 버퍼 데이터를 모든 클라이언트에게 브로드캐스트
    SharedPtr<SendBuffer> sendBuf = gSendChunkPool->Alloc(1024);
    BufferWriter writer(sendBuf->GetBuffer(), sendBuf->GetAllocSize());
    writer.Write(buffer, numBytes);
    sendBuf->OnWritten(numBytes);
    // gClientManager->MakeJob(&ClientSessionManager::Broadcast, std::move(sendBuf));
    gRoom->MakeJob(&Room::Broadcast, std::move(sendBuf));

    return numBytes;
}

void ClientSession::OnSent(Int64 numBytes)
{}

void ClientSessionManager::AddSession(SharedPtr<ClientSession> client)
{
    mClients.insert(client);
}

void ClientSessionManager::RemoveSession(SharedPtr<ClientSession> client)
{
    auto it = mClients.find(client);
    if (it != mClients.end())
    {
        mClients.erase(it);
    }
}

void ClientSessionManager::Broadcast(SharedPtr<SendBuffer> buffer)
{
    for (auto& client : mClients)
    {
        client->Send(buffer);
    }
}

SharedPtr<ClientSessionManager>     gClientManager = std::make_shared<ClientSessionManager>();
