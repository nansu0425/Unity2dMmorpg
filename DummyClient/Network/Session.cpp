/*    DummyClient/Network/Session.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Network/Session.h"
#include "GameContent/Chat/Room.h"

SharedPtr<Room>     gRoom = std::make_shared<Room>();

void ServerSession::OnConnected()
{
    gLogger->Info(TEXT_8("Session[{}]: Connected to server"), GetId());
    // 방에 입장
    gRoom->MakeJob(&Room::Enter, GetSession());
}

void ServerSession::OnDisconnected(String8 cause)
{
    gLogger->Warn(TEXT_8("Session[{}]: Disconnected from server: {}"), GetId(), cause);
    // 방에서 퇴장
    gRoom->MakeJob(&Room::Leave, GetSession());
}

Int64 ServerSession::OnReceived(const Byte* buffer, Int64 numBytes)
{
    gLogger->Debug(TEXT_8("Session[{}]: Received {} bytes"), GetId(), numBytes);
    return numBytes;
}

void ServerSession::OnSent(Int64 numBytes)
{}
