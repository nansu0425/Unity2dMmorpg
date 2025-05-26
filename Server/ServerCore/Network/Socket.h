/*    ServerCore/Network/Socket.h    */

#pragma once

namespace core
{
    class NetAddress;
    class Session;
    struct ConnectEvent;
    struct DisconnectEvent;
    struct AcceptEvent;
    struct ReceiveEvent;
    struct SendEvent;

    /**
     * SocketUtils - Windows 소켓 API 래핑 유틸리티 클래스
     *
     * Windows 소켓 프로그래밍을 간소화하기 위한 정적 유틸리티 함수 모음입니다.
     * 기본적인 소켓 작업부터 확장 비동기 소켓 작업(ConnectEx, DisconnectEx, AcceptEx)까지
     * 통합적인 인터페이스를 제공합니다.
     *
     * 주요 기능:
     * - 소켓 라이브러리 초기화 및 정리
     * - 소켓 생성 및 설정
     * - 다양한 소켓 옵션 조정 (Linger, NoDelay, 버퍼 크기 등)
     * - 주소 바인딩 및 연결 수신 대기
     * - 비동기 소켓 작업 (연결, 해제, 수락, 송수신)
     * - 모든 작업에 대한 표준화된 오류 처리
     *
     * 반환 값 규칙:
     * - SUCCESS(0): 작업 성공
     * - 0이 아닌 값: Windows Socket 오류 코드 (WSAGetLastError() 반환 값)
     */
    class SocketUtils
    {
    public:
        static void                 Init();
        static void                 Cleanup();

        static Int64                BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* function);
        static Int64                CreateSocket(SOCKET& socket);

        static Int64                SetLinger(SOCKET socket, UInt16 onOff, UInt16 lingerTime);
        static Int64                SetReuseAddress(SOCKET socket, Bool enable);
        static Int64                SetNoDelay(SOCKET socket, Bool enable);
        static Int64                SetReceiveBufferSize(SOCKET socket, Int64 size);
        static Int64                SetSendBufferSize(SOCKET socket, Int64 size);
        static Int64                SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);

        static Int64                BindAddress(SOCKET socket, const NetAddress& address);
        static Int64                BindAnyAddress(SOCKET socket, UInt16 port);
        static Int64                Listen(SOCKET socket, Int32 backlog = SOMAXCONN);
        static Int64                CloseSocket(SOCKET& socket);

        static Int64                ConnectAsync(SOCKET socket, const NetAddress& address, Int64* numBytes, ConnectEvent* event);
        static Int64                DisconnectAsync(SOCKET socket, Int64 flags, DisconnectEvent* event);
        static Int64                AcceptAsync(SOCKET listenSocket, SOCKET acceptSocket, Byte* buffer, Int64* numBytes, AcceptEvent* event);
        static Int64                ReceiveAsync(SOCKET socket, WSABUF* buffer, Int64* numBytes, Int64* flags, ReceiveEvent* event);
        static Int64                SendAsync(SOCKET socket, WSABUF* buffers, Int64 bufferCount, Int64* numBytes, SendEvent* event);

    private:
        static LPFN_CONNECTEX       sConnectEx;
        static LPFN_DISCONNECTEX    sDisconnectEx;
        static LPFN_ACCEPTEX        sAcceptEx;
    };

    /**
     * 소켓 옵션 설정 함수
     *
     * 지정된 소켓에 옵션 값을 설정합니다.
     * 템플릿 함수로 다양한 타입의 옵션 값을 처리할 수 있습니다.
     *
     * @param socket 설정할 소켓
     * @param level 옵션 수준 (SOL_SOCKET, IPPROTO_TCP 등)
     * @param optName 옵션 이름
     * @param optVal 설정할 옵션 값
     * @return SUCCESS 성공 시, 오류 코드 실패 시
     */
    template<typename T>
    static Int64 SetSocketOpt(SOCKET socket, Int32 level, Int32 optName, const T& optVal)
    {
        if (SOCKET_ERROR == ::setsockopt(socket, level, optName, reinterpret_cast<const char*>(&optVal), sizeof_32(optVal)))
        {
            return ::WSAGetLastError();
        }

        return SUCCESS;
    }
} // namespace core
