/*    ServerEngine/Network/Service.h    */

#pragma once

#include "ServerEngine/Network/Address.h"
#include "ServerEngine/Io/Dispatcher.h"

class Session;
class Listener;
class SendMessageBuilder;

enum class ServiceType
{
    Server,
    Client,
};

using SessionFactory = Function<SharedPtr<Session>(void)>;

class Service
    : public std::enable_shared_from_this<Service>
{
public:
    struct Config
    {
        NetAddress                      address;
        SharedPtr<IoEventDispatcher>    ioEventDispatcher;
        SessionFactory                  sessionFactory;
        Int64                           maxSessionCount = 1;
    };

public:     // 외부에서 호출하는 함수
    Service(ServiceType type, const Config& config);
    virtual ~Service();

    virtual Int64                   Run() = 0;
    virtual void                    Stop() = 0;

    SharedPtr<Session>              CreateSession();
    Int64                           AddSession(SharedPtr<Session> session);
    Int64                           RemoveSession(SharedPtr<Session> session);
    void                            Broadcast(SharedPtr<SendMessageBuilder> message); // Broadcast 중 연결이 끊기면 세션 제거 과정에서 데드락 발생 가능    

    Bool                            CanRun() const { return mConfig.sessionFactory != nullptr; }
    void                            SetSessionFactory(SessionFactory factory) { mConfig.sessionFactory = std::move(factory); }
    Int64                           GetCurrentSessionCount() const { return mSessionCount; }
    Int64                           GetMaxSessionCount() const { return mConfig.maxSessionCount; }
    ServiceType                     GetType() const { return mType; }
    const NetAddress&               GetAddress() const { return mConfig.address; }
    SharedPtr<IoEventDispatcher>    GetIoEventDispatcher() const { return mConfig.ioEventDispatcher; }

protected:
    ServiceType                     mType;
    Config                          mConfig;

    RW_LOCK;
    HashSet<SharedPtr<Session>>     mSessions;
    Int64                           mSessionCount = 0;  
};

class ClientService
    : public Service
{
public:
    ClientService(const Config& config);

    virtual Int64       Run() override;
    virtual void        Stop() override;
};

class ServerService
    : public Service
{
public:
    ServerService(const Config& config);

    virtual Int64       Run() override;
    virtual void        Stop() override;

protected:
    SharedPtr<Listener>     mListener;
};
