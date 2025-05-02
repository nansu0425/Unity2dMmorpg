 /*    GameServer/Network/Session.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/Session.h"
#include "GameServer/Network/SessionManager.h"
#include "GameServer/Network/Message.h"

void GameSession::OnConnected()
{
    gLogger->Info(TEXT_8("Connected to client"));

    // 세션 매니저에 세션 추가
    gSessionManager.AddSession(std::static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected(String16 cause)
{
    gLogger->Warn(TEXT_16("Disconnected from client: {}"), cause);

    // 세션 매니저에서 세션 제거
    gSessionManager.RemoveSession(std::static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnReceived(ReceiveMessage message)
{
    // 메시지 처리
    gMessageHandlerManager.HandleMessage(GetSharedPtr(), message);
}

void GameSession::OnSent(Int64 numBytes)
{
    // 송신 처리
    gLogger->Debug(TEXT_8("Sent {} bytes to client"), numBytes);
}
