/*    ServerCore/Network/Socket.cpp    */

#include "ServerCore/Pch.h"
#include "ServerCore/Io/Event.h"
#include "ServerCore/Network/Socket.h"
#include "ServerCore/Network/Address.h"
#include "ServerCore/Network/Session.h"

namespace core
{
    /**
     * Winsock 라이브러리 초기화
     *
     * Winsock 2.2 라이브러리를 초기화하고 필수 확장 함수들의 포인터를 획득합니다.
     * 시스템은 반드시 네트워크 작업 전에 이 함수를 한 번 호출해야 합니다.
     * 초기화 실패 시 프로그램이 크래시됩니다.
     */
    void SocketUtils::Init()
    {
        WSADATA wsaData;
        ASSERT_CRASH(SUCCESS == ::WSAStartup(MAKEWORD(2, 2), OUT & wsaData), "WSA_STARTUP_FAILED");
        // 런타임에 주소를 가져온다
        SOCKET dummySocket = INVALID_SOCKET;
        CreateSocket(OUT dummySocket);
        ASSERT_CRASH(SUCCESS == BindWindowsFunction(dummySocket, WSAID_CONNECTEX, OUT reinterpret_cast<LPVOID*>(&sConnectEx)), "CONNECTEX_BIND_FAILED");
        ASSERT_CRASH(SUCCESS == BindWindowsFunction(dummySocket, WSAID_DISCONNECTEX, OUT reinterpret_cast<LPVOID*>(&sDisconnectEx)), "DISCONNECTEX_BIND_FAILED");
        ASSERT_CRASH(SUCCESS == BindWindowsFunction(dummySocket, WSAID_ACCEPTEX, OUT reinterpret_cast<LPVOID*>(&sAcceptEx)), "ACCEPTEX_BIND_FAILED");
        CloseSocket(OUT dummySocket);
    }

    /**
     * Winsock 라이브러리 정리
     *
     * 사용이 끝난 Winsock 라이브러리 리소스를 해제합니다.
     * 프로그램 종료 전 반드시 호출해야 합니다.
     */
    void SocketUtils::Cleanup()
    {
        ::WSACleanup();
    }

    /**
     * Windows 소켓 확장 함수 바인딩
     *
     * 지정된 GUID에 해당하는 Windows 소켓 확장 함수의 포인터를 획득합니다.
     * ConnectEx, DisconnectEx, AcceptEx와 같은 함수 포인터를 런타임에 로드하는 데 사용됩니다.
     *
     * @param socket 함수 포인터를 가져올 소켓
     * @param guid 요청할 함수의 GUID
     * @param function 함수 포인터를 저장할 포인터
     * @return SUCCESS 성공 시, 오류 코드 실패 시
     */
    Int64 SocketUtils::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* function)
    {
        DWORD bytesReturned = 0;
        if (SOCKET_ERROR == ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), function, sizeof(*function), OUT & bytesReturned, nullptr, nullptr))
        {
            return ::WSAGetLastError();
        }

        return SUCCESS;
    }

    /**
     * 소켓 생성
     *
     * 오버랩드 IO를 지원하는 TCP 소켓을 생성합니다.
     *
     * @param socket [OUT] 생성된 소켓을 저장할 참조
     * @return SUCCESS 성공 시, 오류 코드 실패 시
     */
    Int64 SocketUtils::CreateSocket(SOCKET& socket)
    {
        socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
        if (INVALID_SOCKET == socket)
        {
            return ::WSAGetLastError();
        }

        return SUCCESS;
    }

    /**
     * Linger 옵션 설정
     *
     * 소켓 닫기 동작을 제어하는 Linger 옵션을 설정합니다.
     * onOff가 true이면, 소켓을 닫을 때 지정된 시간(초) 동안 대기합니다.
     * onOff가 false이면, 소켓을 즉시 닫습니다.
     *
     * @param socket 설정할 소켓
     * @param onOff 옵션 활성화 여부 (0: 비활성화, 1: 활성화)
     * @param lingerTime 대기 시간(초)
     * @return SUCCESS 성공 시, 오류 코드 실패 시
     */
    Int64 SocketUtils::SetLinger(SOCKET socket, UInt16 onOff, UInt16 lingerTime)
    {
        LINGER linger = {};
        linger.l_onoff = onOff;
        linger.l_linger = lingerTime;

        return SetSocketOpt(socket, SOL_SOCKET, SO_LINGER, linger);
    }

    /**
     * 주소 재사용 옵션 설정
     *
     * SO_REUSEADDR 옵션을 설정하여 소켓이 이전에 사용한 주소를 재사용할 수 있게 합니다.
     * 서버 소켓 재시작 시 "Address already in use" 오류를 방지합니다.
     *
     * @param socket 설정할 소켓
     * @param enable true: 주소 재사용 활성화, false: 비활성화
     * @return SUCCESS 성공 시, 오류 코드 실패 시
     */
    Int64 SocketUtils::SetReuseAddress(SOCKET socket, Bool enable)
    {
        return SetSocketOpt(socket, SOL_SOCKET, SO_REUSEADDR, enable);
    }

    /**
     * Nagle 알고리즘 비활성화 옵션 설정
     *
     * TCP_NODELAY 옵션을 설정하여 Nagle 알고리즘을 제어합니다.
     * 활성화하면 작은 패킷을 즉시 전송하여 지연을 줄이지만 네트워크 효율은 감소할 수 있습니다.
     *
     * @param socket 설정할 소켓
     * @param enable true: Nagle 알고리즘 비활성화, false: 활성화
     * @return SUCCESS 성공 시, 오류 코드 실패 시
     */
    Int64 SocketUtils::SetNoDelay(SOCKET socket, Bool enable)
    {
        return SetSocketOpt(socket, SOL_SOCKET, TCP_NODELAY, enable);
    }

    /**
     * 수신 버퍼 크기 설정
     *
     * 소켓의 수신 버퍼 크기를 설정합니다.
     * 대용량 데이터 수신 시 성능 향상을 위해 사용할 수 있습니다.
     *
     * @param socket 설정할 소켓
     * @param size 설정할 버퍼 크기(바이트)
     * @return SUCCESS 성공 시, 오류 코드 실패 시
     */
    Int64 SocketUtils::SetReceiveBufferSize(SOCKET socket, Int64 size)
    {
        return SetSocketOpt(socket, SOL_SOCKET, SO_RCVBUF, size);
    }

    /**
     * 송신 버퍼 크기 설정
     *
     * 소켓의 송신 버퍼 크기를 설정합니다.
     * 대용량 데이터 송신 시 성능 향상을 위해 사용할 수 있습니다.
     *
     * @param socket 설정할 소켓
     * @param size 설정할 버퍼 크기(바이트)
     * @return SUCCESS 성공 시, 오류 코드 실패 시
     */
    Int64 SocketUtils::SetSendBufferSize(SOCKET socket, Int64 size)
    {
        return SetSocketOpt(socket, SOL_SOCKET, SO_SNDBUF, size);
    }

    /**
     * Accept 소켓 컨텍스트 업데이트
     *
     * AcceptEx로 생성된 소켓의 속성을 리스닝 소켓과 일치하도록 업데이트합니다.
     * AcceptEx 호출 후 반드시 호출해야 합니다.
     *
     * @param socket AcceptEx로 생성된 소켓
     * @param listenSocket 리스닝 소켓
     * @return SUCCESS 성공 시, 오류 코드 실패 시
     */
    Int64 SocketUtils::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket)
    {
        return SetSocketOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
    }

    /**
     * 소켓에 주소 바인딩
     *
     * 지정된 주소를 소켓에 바인딩합니다.
     * 서버 소켓에서 특정 IP 및 포트에서 수신하기 위해 사용합니다.
     *
     * @param socket 바인딩할 소켓
     * @param address 바인딩할 네트워크 주소
     * @return SUCCESS 성공 시, 오류 코드 실패 시
     */
    Int64 SocketUtils::BindAddress(SOCKET socket, const NetAddress& address)
    {
        if (SOCKET_ERROR == ::bind(socket, reinterpret_cast<const SOCKADDR*>(&address.GetAddress()), sizeof(SOCKADDR_IN)))
        {
            return ::WSAGetLastError();
        }

        return SUCCESS;
    }

    /**
     * 소켓에 임의 주소 바인딩
     *
     * INADDR_ANY를 사용하여 모든 네트워크 인터페이스에서 지정된 포트로 수신하도록 바인딩합니다.
     * 클라이언트 소켓이 특정 로컬 포트를 사용하도록 할 때도 활용할 수 있습니다.
     *
     * @param socket 바인딩할 소켓
     * @param port 바인딩할 포트 번호 (0: 시스템이 자동 할당)
     * @return SUCCESS 성공 시, 오류 코드 실패 시
     */
    Int64 SocketUtils::BindAnyAddress(SOCKET socket, UInt16 port)
    {
        SOCKADDR_IN addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = ::htons(port);

        if (SOCKET_ERROR == ::bind(socket, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)))
        {
            return ::WSAGetLastError();
        }

        return SUCCESS;
    }

    /**
     * 연결 요청 대기 시작
     *
     * 바인딩된 소켓을 리스닝 상태로 전환하여 클라이언트 연결을 수신할 수 있게 합니다.
     * 서버 소켓 초기화의 마지막 단계입니다.
     *
     * @param socket 리스닝할 소켓
     * @param backlog 대기열에 허용할 최대 연결 요청 수 (기본값: SOMAXCONN)
     * @return SUCCESS 성공 시, 오류 코드 실패 시
     */
    Int64 SocketUtils::Listen(SOCKET socket, Int32 backlog)
    {
        if (SOCKET_ERROR == ::listen(socket, backlog))
        {
            return ::WSAGetLastError();
        }

        return SUCCESS;
    }

    /**
     * 소켓 닫기
     *
     * 지정된 소켓을 안전하게 닫고 INVALID_SOCKET으로 초기화합니다.
     * 소켓 사용 완료 후 반드시 호출하여 리소스 누수를 방지해야 합니다.
     *
     * @param socket [IN/OUT] 닫을 소켓 참조
     * @return SUCCESS 성공 시, 오류 코드 실패 시
     */
    Int64 SocketUtils::CloseSocket(SOCKET& socket)
    {
        if (INVALID_SOCKET == socket)
        {
            return SUCCESS;
        }

        if (SOCKET_ERROR == ::closesocket(socket))
        {
            return ::WSAGetLastError();
        }
        socket = INVALID_SOCKET;

        return SUCCESS;
    }

    /**
     * 비동기 연결 요청
     *
     * ConnectEx API를 사용하여 비동기적으로 서버에 연결을 시도합니다.
     * 연결 완료는 IOCP를 통해 통지됩니다.
     *
     * @param socket 연결할 소켓
     * @param address 연결할 서버 주소
     * @param numBytes [OUT] 전송된 바이트 수를 저장할 포인터
     * @param event 연결 완료 처리를 위한 이벤트 객체
     * @return SUCCESS 성공 시, WSA_IO_PENDING 비동기 요청 중, 기타 오류 코드 실패 시
     */
    Int64 SocketUtils::ConnectAsync(SOCKET socket, const NetAddress& address, Int64* numBytes, ConnectEvent* event)
    {
        if (FALSE == sConnectEx(socket, reinterpret_cast<const SOCKADDR*>(&address.GetAddress()), sizeof(SOCKADDR_IN), nullptr, 0, OUT reinterpret_cast<LPDWORD>(numBytes), event))
        {
            return ::WSAGetLastError();
        }

        return SUCCESS;
    }

    /**
     * 비동기 연결 해제
     *
     * DisconnectEx API를 사용하여 비동기적으로 연결을 해제합니다.
     * 연결 해제 완료는 IOCP를 통해 통지됩니다.
     *
     * @param socket 해제할 소켓
     * @param flags 연결 해제 옵션 (TF_REUSE_SOCKET: 소켓 재사용)
     * @param event 연결 해제 완료 처리를 위한 이벤트 객체
     * @return SUCCESS 성공 시, WSA_IO_PENDING 비동기 요청 중, 기타 오류 코드 실패 시
     */
    Int64 SocketUtils::DisconnectAsync(SOCKET socket, Int64 flags, DisconnectEvent* event)
    {
        if (FALSE == sDisconnectEx(socket, event, static_cast<DWORD>(flags), 0))
        {
            return ::WSAGetLastError();
        }

        return SUCCESS;
    }

    /**
     * 비동기 연결 수락
     *
     * AcceptEx API를 사용하여 비동기적으로 클라이언트 연결을 수락합니다.
     * 수락 완료는 IOCP를 통해 통지됩니다.
     *
     * @param listenSocket 리스닝 소켓
     * @param acceptSocket 수락할 소켓
     * @param buffer 초기 수신 데이터를 저장할 버퍼
     * @param numBytes [OUT] 수신된 바이트 수를 저장할 포인터
     * @param event 수락 완료 처리를 위한 이벤트 객체
     * @return SUCCESS 성공 시, WSA_IO_PENDING 비동기 요청 중, 기타 오류 코드 실패 시
     */
    Int64 SocketUtils::AcceptAsync(SOCKET listenSocket, SOCKET acceptSocket, Byte* buffer, Int64* numBytes, AcceptEvent* event)
    {
        if (FALSE == sAcceptEx(listenSocket, acceptSocket, buffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, reinterpret_cast<LPDWORD>(&numBytes), static_cast<LPOVERLAPPED>(event)))
        {
            return ::WSAGetLastError();
        }

        return SUCCESS;
    }

    /**
     * 비동기 데이터 수신
     *
     * WSARecv API를 사용하여 비동기적으로 데이터를 수신합니다.
     * 수신 완료는 IOCP를 통해 통지됩니다.
     *
     * @param socket 수신할 소켓
     * @param buffer 데이터를 수신할 버퍼 구조체
     * @param numBytes [OUT] 수신된 바이트 수를 저장할 포인터
     * @param flags [IN/OUT] 수신 플래그
     * @param event 수신 완료 처리를 위한 이벤트 객체
     * @return SUCCESS 성공 시, WSA_IO_PENDING 비동기 요청 중, 기타 오류 코드 실패 시
     */
    Int64 SocketUtils::ReceiveAsync(SOCKET socket, WSABUF* buffer, Int64* numBytes, Int64* flags, ReceiveEvent* event)
    {
        if (SOCKET_ERROR == ::WSARecv(socket, buffer, 1, OUT reinterpret_cast<LPDWORD>(numBytes), OUT reinterpret_cast<LPDWORD>(flags), event, nullptr))
        {
            return ::WSAGetLastError();
        }

        return SUCCESS;
    }

    /**
     * 비동기 데이터 송신
     *
     * WSASend API를 사용하여 비동기적으로 데이터를 송신합니다.
     * 송신 완료는 IOCP를 통해 통지됩니다.
     *
     * @param socket 송신할 소켓
     * @param buffers 송신할 데이터가 있는 버퍼 배열
     * @param bufferCount 버퍼 배열의 크기
     * @param numBytes [OUT] 송신된 바이트 수를 저장할 포인터
     * @param event 송신 완료 처리를 위한 이벤트 객체
     * @return SUCCESS 성공 시, WSA_IO_PENDING 비동기 요청 중, 기타 오류 코드 실패 시
     */
    Int64 SocketUtils::SendAsync(SOCKET socket, WSABUF* buffers, Int64 bufferCount, Int64* numBytes, SendEvent* event)
    {
        if (SOCKET_ERROR == ::WSASend(socket, buffers, static_cast<DWORD>(bufferCount), OUT reinterpret_cast<LPDWORD>(numBytes), 0, event, nullptr))
        {
            return ::WSAGetLastError();
        }

        return SUCCESS;
    }

    // 확장 소켓 함수 포인터 초기화
    LPFN_CONNECTEX           SocketUtils::sConnectEx = nullptr;
    LPFN_DISCONNECTEX        SocketUtils::sDisconnectEx = nullptr;
    LPFN_ACCEPTEX            SocketUtils::sAcceptEx = nullptr;
}
