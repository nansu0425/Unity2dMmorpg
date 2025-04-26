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
    // std::wcout << TEXT_16("Disconnected: ") << cause << std::endl;
    // 세션 매니저에서 세션 제거
    gSessionManager.RemoveSession(std::static_pointer_cast<GameSession>(shared_from_this()));
}

Int64 GameSession::OnReceived(Byte* buffer, Int64 numBytes)
{
    gLogger->Info(TEXT_16("Received: {} bytes"), numBytes);
    // std::wcout << TEXT_16("Received: ") << numBytes << TEXT_16(" bytes") << std::endl;
    // 수신한 데이터를 모든 세션에 브로드캐스트
    SharedPtr<SendBuffer> sendBuffer = std::make_shared<SendBuffer>(numBytes);
    sendBuffer->CopyData(buffer, numBytes);
    gSessionManager.Broadcast(sendBuffer);

    return numBytes;
}

void GameSession::OnSent(Int64 numBytes)
{
    // 전송 완료 처리
    gLogger->Info(TEXT_16("Sent: {} bytes"), numBytes);
    // std::wcout << TEXT_16("Sent: ") << numBytes << TEXT_16(" bytes") << std::endl;
}
