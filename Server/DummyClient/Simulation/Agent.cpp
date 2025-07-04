/*    DummyClient/Simulation/Agent.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Simulation/Agent.h"

namespace dummy
{
    Agent::Agent(AgentId id, const SharedPtr<ServerSession>& session)
        : mId(id), mSession(session)
    {}

    SharedPtr<Agent> AgentManager::AddAgent(const SharedPtr<ServerSession>& session)
    {
        static Atomic<AgentId> sNextAgentId = 1;

        SharedPtr<Agent> agent = std::make_shared<Agent>(sNextAgentId.fetch_add(1), session);
        {
            WRITE_GUARD;

            mAgents[agent->GetId()] = agent;
            mSessionToAgentIdMap[session->GetId()] = agent->GetId();
        }

        core::gLogger->Info(TEXT_8("Agent[{}] added with session ID [{}]"), agent->GetId(), session->GetId());

        return agent;
    }

    SharedPtr<Agent> AgentManager::FindAgent(AgentId agentId)
    {
        SharedPtr<Agent> agent;
        {
            READ_GUARD;

            auto it = mAgents.find(agentId);
            if (it != mAgents.end())
            {
                agent = it->second;
            }
        }

        return agent;
    }

    SharedPtr<Agent> AgentManager::FindAgentBySessionId(Int64 sessionId)
    {
        SharedPtr<Agent> agent;
        {
            READ_GUARD;

            auto agentIdIt = mSessionToAgentIdMap.find(sessionId);
            if (agentIdIt != mSessionToAgentIdMap.end())
            {
                auto agentIt = mAgents.find(agentIdIt->second);
                if (agentIt != mAgents.end())
                {
                    agent = agentIt->second; // 에이전트 찾기
                }
            }
        }

        return agent;
    }

    Bool AgentManager::RemoveAgent(AgentId agentId)
    {
        WRITE_GUARD;

        auto it = mAgents.find(agentId);
        if (it == mAgents.end())
        {
            return false; // 에이전트가 존재하지 않음
        }

        mSessionToAgentIdMap.erase(it->second->GetSessionId());
        mAgents.erase(it);

        core::gLogger->Info(TEXT_8("Agent[{}] removed"), agentId);

        return true; // 성공적으로 제거됨
    }
}
