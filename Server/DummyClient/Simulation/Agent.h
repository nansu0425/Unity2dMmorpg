/*    DummyClient/Simulation/Agent.h    */

#pragma once

#include "DummyClient/Network/Session.h"

namespace dummy
{

    using AgentId = Int64;

    class Agent
    {
    public:
        Agent(AgentId id, const SharedPtr<ServerSession>& session);

        AgentId GetId() const { return mId; }
        Int64 GetSessionId() const { return mSession->GetId(); }

    private:
        const AgentId mId;
        SharedPtr<ServerSession> mSession;
    };

    class AgentManager
    {
    public:
        static AgentManager& GetInstance()
        {
            static AgentManager sInstance;
            return sInstance;
        }

        SharedPtr<Agent> AddAgent(const SharedPtr<ServerSession>& session);
        SharedPtr<Agent> FindAgent(AgentId agentId);
        SharedPtr<Agent> FindAgentBySessionId(Int64 sessionId);
        Bool RemoveAgent(AgentId agentId);

    private:
        AgentManager() = default;

    private:
        RW_LOCK;
        HashMap<AgentId, SharedPtr<Agent>> mAgents;
        HashMap<Int64, AgentId> mSessionToAgentIdMap;
    };
}
