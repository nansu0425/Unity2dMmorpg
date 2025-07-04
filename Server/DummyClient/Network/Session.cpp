/*    DummyClient/Network/Session.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Network/Session.h"
#include "DummyClient/Packet/Handler.h"
#include "Protocol/Packet/Utils.h"
#include "DummyClient/Core/Loop.h"
#include "DummyClient/Simulation/Agent.h"

namespace dummy
{
    ServerSession::~ServerSession()
    {
        core::gLogger->Info(TEXT_8("Session[{}]: Destroyed"), GetId());
    }

    void ServerSession::OnConnected()
    {
        core::gLogger->Info(TEXT_8("Session[{}]: Connected to server"), GetId());

        // 에이전트 추가
        SharedPtr<Agent> agent = AgentManager::GetInstance().AddAgent(GetServerSession());

        // 방 입장 요청 전송
        proto::ClientToWorld_EnterRoom payload;
        payload.set_id(agent->GetId());
        payload.set_password(TEXT_8("1234"));
        proto::PacketUtils::Send(GetSession(), payload);
    }

    void ServerSession::OnDisconnected(String8 cause)
    {
        core::gLogger->Warn(TEXT_8("Session[{}]: Disconnected from server: {}"), GetId(), cause);

        // 에이전트 제거
        SharedPtr<Agent> agent = AgentManager::GetInstance().FindAgent(GetSession()->GetId());
        Bool result = AgentManager::GetInstance().RemoveAgent(agent->GetId());
        if (!result)
        {
            core::gLogger->Error(TEXT_8("Session[{}]: Failed to remove agent[{}]"), GetId(), agent->GetId());
        }
    }

    Int64 ServerSession::OnReceived(const Byte* buffer, Int64 numBytes)
    {
        return dummy::Loop::GetInstance().PushPackets(GetSession(), buffer, numBytes);
    }

    void ServerSession::OnSent(Int64 numBytes)
    {}
} // namespace dummy
