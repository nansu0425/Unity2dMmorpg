/*    DummyClient/Packet/Handler.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Packet/Handler.h"
#include "DummyClient/Network/Session.h"
#include "DummyClient/Simulation/Agent.h"

namespace dummy
{
    Bool ToClient_PacketHandler::Handle_WorldToClient_EnterRoom(const SharedPtr<core::Session>& owner, const proto::WorldToClient_EnterRoom& payload)
    {
        if (!payload.success())
        {
            core::gLogger->Error(TEXT_8("Session[{}]: Failed to enter room"), owner->GetId());
            return false;
        }

        // 에전전트 찾기
        SharedPtr<Agent> agent = AgentManager::GetInstance().FindAgentBySessionId(owner->GetId());
        if (!agent)
        {
            core::gLogger->Error(TEXT_8("Session[{}]: Agent not found"), owner->GetId());
            return false;
        }

        // 에이전트 ID 확인
        if (agent->GetId() != payload.id())
        {
            core::gLogger->Error(TEXT_8("Session[{}]: Agent ID mismatch"), owner->GetId());
            return false;
        }

        return true;
    }

    Bool ToClient_PacketHandler::Handle_WorldToClient_Chat(const SharedPtr<core::Session>& owner, const proto::WorldToClient_Chat& payload)
    {
        return true;
    }
} // namespace dummy
