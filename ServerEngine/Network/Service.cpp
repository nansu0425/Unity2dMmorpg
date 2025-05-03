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

Int64 Service::AddSession(SharedPtr<Session> session)
{
    WRITE_GUARD;

    // 최대 세션 수에 도달한 경우
    if (mSessionCount >= mConfig.maxSessionCount)
    {
        gLogger->Error(TEXT_8("Max session count reached"));
        return FAILURE;
    }

    // 세션 추가
    if (!mSessions.insert(session).second)
    {
        gLogger->Error(TEXT_8("Session already exists: session"));
        return FAILURE;
    }
    // 세션 카운트 증가
    ++mSessionCount;

    return SUCCESS;
}

Int64 Service::RemoveSession(SharedPtr<Session> session)
{
    WRITE_GUARD;
    // 세션 제거
    if (mSessions.erase(session) == 0)
    {
        gLogger->Error(TEXT_8("Session not found"));
        return FAILURE;
    }
    // 세션 카운트 감소
    --mSessionCount;

    return SUCCESS;
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
    result = mListener->StartAccept(std::move(self));

    return result;
}

void ServerService::Stop()
{
    // TODO
}
