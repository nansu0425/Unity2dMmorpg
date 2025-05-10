 /*    GameServer/Network/Session.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/Session.h"
#include "GameServer/Network/Message.h"
#include "GameServer/Content/Room.h"

//void ClientSession::OnConnected()
//{
//    gLogger->Info(TEXT_8("Connected to client"));
//
//    // 세션 매니저에 세션 추가
//    gClientManager.AddSession(std::static_pointer_cast<ClientSession>(shared_from_this()));
//}
//
//void ClientSession::OnDisconnected(String8 cause)
//{
//    gLogger->Warn(TEXT_8("Disconnected from client: {}"), cause);
//
//    // 세션 매니저에서 세션 제거
//    gClientManager.RemoveSession(std::static_pointer_cast<ClientSession>(shared_from_this()));
//
//    // 룸에서 세션의 현재 플레이어 제거
//    if (mCurrentPlayerIdx > -1)
//    {
//        GetPlayerRoom()->MakeJob(&Room::Leave, GetPlayer(mCurrentPlayerIdx));
//    }
//    mCurrentPlayerIdx = -1;
//
//    // Session -> Player 참조 해제
//    mPlayers.clear();
//}
//
//void ClientSession::OnReceived(ReceiveMessage message)
//{
//    // 메시지 처리
//    gMessageHandlerManager.HandleMessage(GetSharedPtr(), message);
//}
//
//void ClientSession::OnSent(Int64 numBytes)
//{}

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

Int64 ClientSession::OnReceived(const Byte* buffer, Int64 numBytes)
{
    gLogger->Debug(TEXT_8("Received {} bytes"), numBytes);

    // 수신 버퍼 데이터를 모든 클라이언트에게 브로드캐스트
    SharedPtr<SendBuffer> sendBuf = gSendChunkPool->Alloc(1024);
    BufferWriter writer(sendBuf->GetBuffer(), sendBuf->GetAllocSize());
    writer.Write(buffer, numBytes);
    sendBuf->OnWritten(numBytes);
    gClientManager.Broadcast(std::move(sendBuf));

    return numBytes;
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

void ClientSessionManager::Broadcast(SharedPtr<SendBuffer> buffer)
{
    WRITE_GUARD;
    for (auto& client : mClients)
    {
        client->Send(buffer);
    }
}

//void ClientSessionManager::Broadcast(SharedPtr<SendMessageBuilder> message)
//{
//    WRITE_GUARD;
//    for (auto& client : mClients)
//    {
//        if (client->IsLogined())
//        {
//            client->Send(message);
//        }
//    }
//}

ClientSessionManager gClientManager;
