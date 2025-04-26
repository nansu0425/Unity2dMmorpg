 /*    GameServer/Network/Session.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/Session.h"
#include "GameServer/Network/SessionManager.h"

void GameSession::OnConnected()
{
    gLogger->Info(TEXT_16("Connected to client"));

    // 세션 매니저에 세션 추가
    gSessionManager.AddSession(std::static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected(String16 cause)
{
    gLogger->Warn(TEXT_16("Disconnected from client: {}"), cause);

    // 세션 매니저에서 세션 제거
    gSessionManager.RemoveSession(std::static_pointer_cast<GameSession>(shared_from_this()));
}

Int64 GameSession::OnReceived(Byte* buffer, Int64 numBytes)
{
    gLogger->Info(TEXT_16("Received: {} bytes"), numBytes);

    // 수신 데이터를 송신 버퍼에 복사
    SharedPtr<SendBuffer> sendBuffer = gSendBufferManager->Open(4096);
    ::memcpy(sendBuffer->GetBuffer(), buffer, numBytes);
    sendBuffer->Close(numBytes);
    // 수신 데이터를 모든 세션에 브로드캐스트
    gSessionManager.Broadcast(sendBuffer);

    return numBytes;
}

void GameSession::OnSent(Int64 numBytes)
{
    gLogger->Info(TEXT_16("Sent: {} bytes"), numBytes);
}
