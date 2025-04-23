/*    ServerEngine/Network/Service.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Network/Service.h"
#include "ServerEngine/Network/Session.h"
#include "ServerEngine/Network/Listener.h"

Service::Service(ServiceType type, const Config& config)
    : mType(type)
    , mConfig(config)
{}

Service::~Service()
{}

SharedPtr<Session> Service::CreateSession()
{
    SharedPtr<Session> session = mConfig.sessionFactory();
    session->SetService(shared_from_this());

    // IoEventDispatcher에 세션 등록
    if (SUCCESS == mConfig.ioDispatcher->Register(session))
    {
        return session;
    }

    return nullptr;
}

void Service::AddSession(SharedPtr<Session> session)
{
    WRITE_GUARD;
    // 세션 추가 후 성공적으로 추가되면 세션 카운트 증가
    if (mSessions.insert(session).second)
    {
        ++mSessionCount;
    }
    else
    {
        std::cerr << "Failed to add session: already exists" << std::endl;
    }
}

void Service::RemoveSession(SharedPtr<Session> session)
{
    WRITE_GUARD;
    // 세션 제거 후 성공적으로 제거되면 세션 카운트 감소
    if (mSessions.erase(session) > 0)
    {
        --mSessionCount;
    }
    else
    {
        std::cerr << "Failed to remove session: not found" << std::endl;
    }
}

ClientService::ClientService(const Config& config)
    : Service(ServiceType::Client, config)
{}

Int64 ClientService::Run()
{
    Int64 result = SUCCESS;

    if (!CanRun())
    {
        result = FAILURE;
        return result;
    }

    for (Int64 i = 0; i < mConfig.maxSessionCount; ++i)
    {
        // 세션 생성
        SharedPtr<Session> session = CreateSession();
        session->SetNetAddress(mConfig.address);
        if (nullptr == session)
        {
            result = FAILURE;
            break;
        }
        // 비동기 연결
        result = session->Connect();
        if (result != SUCCESS)
        {
            break;
        }
    }

    return result;
}

void ClientService::Stop()
{}

ServerService::ServerService(const Config& config)
    : Service(ServiceType::Server, config)
{}

Int64 ServerService::Run()
{
    Int64 result = SUCCESS;

    if (!CanRun())
    {
        result = FAILURE;
        return result;
    }

    // 리스너 생성
    if (nullptr == (mListener = std::make_shared<Listener>()))
    {
        result = FAILURE;
        return result;
    }

    // 리스너 accept 시작
    SharedPtr<ServerService> self = std::static_pointer_cast<ServerService>(shared_from_this());
    if (result = mListener->StartAccept(std::move(self)))
    {
        return result;
    }

    return result;
}

void ServerService::Stop()
{
    // TODO
}
