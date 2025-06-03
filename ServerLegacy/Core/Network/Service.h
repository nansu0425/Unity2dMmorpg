/*    Core/Network/Service.h    */

#pragma once

#include "Core/Network/Address.h"
#include "Core/Io/Dispatcher.h"

namespace core
{
    class Session;
    class Listener;

    enum class ServiceType
    {
        Server,
        Client,
    };

    using SessionFactory = Function<SharedPtr<Session>(void)>;

    /**
     * Service - 네트워크 서비스의 기본 추상 클래스
     *
     * 서버/클라이언트 네트워크 서비스의 공통 기능을 제공하는 추상 기본 클래스입니다.
     * 세션 생성, 관리, 검색 등 네트워크 연결의 기본적인 수명 주기를 처리합니다.
     *
     * 주요 기능:
     * - 세션 생성 및 관리 (추가, 제거, 검색)
     * - 세션 ID 자동 할당 및 관리
     * - 스레드 안전한 세션 컬렉션 관리
     * - IO 이벤트 디스패처 연동
     *
     * 파생 클래스:
     * - ServerService: 서버 측 연결 수신 서비스
     * - ClientService: 클라이언트 측 연결 요청 서비스
     */
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

    public:
        Service(ServiceType type, const Config& config);
        virtual ~Service();

        virtual Int64                   Run() = 0;
        virtual void                    Stop() = 0;

        SharedPtr<Session>              CreateSession();
        Int64                           AddSession(SharedPtr<Session> session);
        Int64                           RemoveSession(SharedPtr<Session> session);
        SharedPtr<Session>              FindSession(Int64 sessionId);

        Bool                            CanRun() const { return mConfig.sessionFactory != nullptr; }
        void                            SetSessionFactory(SessionFactory factory) { mConfig.sessionFactory = std::move(factory); }
        Int64                           GetCurrentSessionCount() const { return mSessionCount; }
        Int64                           GetMaxSessionCount() const { return mConfig.maxSessionCount; }
        ServiceType                     GetType() const { return mType; }
        const NetAddress&               GetAddress() const { return mConfig.address; }
        SharedPtr<IoEventDispatcher>    GetIoEventDispatcher() const { return mConfig.ioEventDispatcher; }

    protected:
        ServiceType     mType;
        Config          mConfig;

        RW_LOCK;
        HashMap<Int64, SharedPtr<Session>>      mSessions;
        Int64                                   mSessionCount = 0;
    };

    /**
     * ClientService - 클라이언트 측 네트워크 서비스 클래스
     *
     * 외부 서버에 대한 연결 요청 및 세션 관리를 담당하는 서비스 클래스입니다.
     * 목표 서버 주소로 다수의 클라이언트 연결을 요청하고 관리합니다.
     *
     * 주요 기능:
     * - 다수의 클라이언트 세션 동시 생성 및 연결
     * - 비동기 연결 요청 처리
     * - 클라이언트 세션 수명 주기 관리
     *
     * 사용 예시:
     * ClientService::Config config;
     * config.address = NetAddress(TEXT_16("127.0.0.1"), 8000);
     * config.maxSessionCount = 100;
     * auto service = std::make_shared<ClientService>(config);
     * service->Run();
     */
    class ClientService
        : public Service
    {
    public:
        ClientService(const Config& config);

        virtual Int64       Run() override;
        virtual void        Stop() override;
    };

    /**
     * ServerService - 서버 측 네트워크 서비스 클래스
     *
     * 클라이언트 연결 요청을 수신하고 세션을 관리하는 서비스 클래스입니다.
     * Listener를 통해 클라이언트 연결을 수락하고 세션을 생성합니다.
     *
     * 주요 기능:
     * - 클라이언트 연결 수신을 위한 Listener 관리
     * - 접속한 클라이언트 세션 생성 및 관리
     * - 세션 수명 주기 관리
     *
     * 사용 예시:
     * ServerService::Config config;
     * config.address = NetAddress(TEXT_16("127.0.0.1"), 8000);
     * config.maxSessionCount = 1000;
     * auto service = std::make_shared<ServerService>(config);
     * service->Run();
     */
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
} // namespace core
