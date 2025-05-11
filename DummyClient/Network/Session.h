/*    DummyClient/Network/Session.h    */

#include "ServerEngine/Network/Session.h"
#include "DummyClient/Network/Protocol/Packet.pb.h"

#pragma once

class ServerSession
    : public Session
{
protected:
    virtual void        OnConnected() override;
    virtual void        OnDisconnected(String8 cause) override;
    virtual Int64       OnReceived(const Byte* buffer, Int64 numBytes) override;
    virtual void        OnSent(Int64 numBytes) override;
};

class ServerSessionManager
    : public JobSerializer
{
public:
    void                AddSession(SharedPtr<ServerSession> server);
    void                RemoveSession(SharedPtr<ServerSession> server); 
    void                Broadcast(SharedPtr<SendBuffer> buffer);

private:
    HashSet<SharedPtr<ServerSession>>   mServers;
};

extern SharedPtr<ServerSessionManager>  gServerManager;
