/*    ServerEngine/Network/Service.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Network/Service.h"
#include "ServerEngine/Network/Session.h"
#include "ServerEngine/Network/Listener.h"

/**
 * Service 생성자
 *
 * 서비스의 타입과 설정으로 새 서비스 인스턴스를 초기화합니다.
 *
 * @param type 서비스 타입 (서버 또는 클라이언트)
 * @param config 서비스 설정 (주소, 디스패처, 세션 팩토리 등)
 */
Service::Service(ServiceType type, const Config& config)
    : mType(type)
    , mConfig(config)
{}

/**
 * Service 소멸자
 *
 * 서비스 리소스를 정리합니다. 파생 클래스에서 필요한 추가 정리 작업을 수행합니다.
 */
Service::~Service()
{}

/**
 * 새 세션 생성
 *
 * 세션 팩토리를 사용하여 새 세션을 생성하고 초기화합니다.
 * 생성된 세션에 고유 ID를 할당하고 IO 이벤트 디스패처에 등록합니다.
 *
 * @return 생성된 세션 (성공시) 또는 nullptr (실패시)
 */
SharedPtr<Session> Service::CreateSession()
{
    static Atomic<Int64> sNextSessionId = 1;

    SharedPtr<Session> session = mConfig.sessionFactory();
    session->SetService(shared_from_this());
    session->SetId(sNextSessionId.fetch_add(1));

    // IoEventDispatcher에 세션 등록
    if (SUCCESS == mConfig.ioEventDispatcher->Register(session))
    {
        return session;
    }

    return nullptr;
}

/**
 * 세션 추가
 *
 * 생성된 세션을 서비스의 세션 맵에 추가합니다.
 * 최대 세션 수를 초과하면 추가를 거부합니다.
 *
 * @param session 추가할 세션
 * @return SUCCESS 성공 시, FAILURE 실패 시
 */
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
    if (!mSessions.insert({session->GetId(), session}).second)
    {
        gLogger->Error(TEXT_8("Session already exists: session"));
        return FAILURE;
    }

    // 세션 카운트 증가
    ++mSessionCount;

    return SUCCESS;
}

/**
 * 세션 제거
 *
 * 지정된 세션을 서비스의 세션 맵에서 제거합니다.
 *
 * @param session 제거할 세션
 * @return SUCCESS 성공 시, FAILURE 세션을 찾을 수 없는 경우
 */
Int64 Service::RemoveSession(SharedPtr<Session> session)
{
    WRITE_GUARD;

    // 세션 제거
    if (mSessions.erase(session->GetId()) == 0)
    {
        gLogger->Error(TEXT_8("Session not found"));
        return FAILURE;
    }

    // 세션 카운트 감소
    --mSessionCount;

    return SUCCESS;
}

/**
 * 세션 검색
 *
 * 지정된 ID의 세션을 찾아 반환합니다.
 *
 * @param sessionId 찾을 세션의 ID
 * @return 찾은 세션 또는 nullptr (없는 경우)
 */
SharedPtr<Session> Service::FindSession(Int64 sessionId)
{
    READ_GUARD;

    // 세션 찾기
    auto it = mSessions.find(sessionId);
    if (it != mSessions.end())
    {
        return it->second;
    }

    return nullptr;
}

/**
 * ClientService 생성자
 *
 * 클라이언트 타입의 서비스를 초기화합니다.
 *
 * @param config 서비스 구성 정보
 */
ClientService::ClientService(const Config& config)
    : Service(ServiceType::Client, config)
{}

/**
 * 클라이언트 서비스 시작
 *
 * 클라이언트 세션을 생성하고 서버에 연결을 시도합니다.
 * 서비스 구성에 지정된 최대 세션 수만큼의 연결을 시도합니다.
 *
 * @return SUCCESS 성공 시, FAILURE 실패 시
 */
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

        // 비동기 연결 요청
        result = session->ConnectAsync();
        if (result != SUCCESS)
        {
            break;
        }
    }

    return result;
}

/**
 * 클라이언트 서비스 중지
 *
 * 클라이언트 서비스를 중지하고 모든 연결을 종료합니다.
 * 현재 구현은 비어 있습니다 (향후 구현 예정).
 */
void ClientService::Stop()
{}

/**
 * ServerService 생성자
 *
 * 서버 타입의 서비스를 초기화합니다.
 *
 * @param config 서비스 구성 정보
 */
ServerService::ServerService(const Config& config)
    : Service(ServiceType::Server, config)
{}

/**
 * 서버 서비스 시작
 *
 * 리스너를 생성하고 클라이언트 연결 수락을 시작합니다.
 *
 * @return SUCCESS 성공 시, FAILURE 실패 시
 */
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

/**
 * 서버 서비스 중지
 *
 * 서버 서비스를 중지하고 모든 연결을 종료합니다.
 * 현재 구현은 비어 있습니다 (향후 구현 예정).
 */
void ServerService::Stop()
{
    // TODO
}
