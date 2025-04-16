/*    ServerEngine/Network/SocketUtils.h    */

#pragma once

class NetAddress;

/*
 * 소켓 처리 유틸리티 클래스
 * 정수를 반환하는 함수가 0이 아닌 값을 반환하면 에러 코드
 */
class SocketUtils
{
public:
    static void                 Init();
    static void                 Cleanup();

    static Int32                BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* function);
    static Int32                CreateSocket(SOCKET& socket);

    static Int32                SetLinger(SOCKET socket, UInt16 onOff, UInt16 lingerTime);
    static Int32                SetReuseAddress(SOCKET socket, Bool enable);
    static Int32                SetNoDelay(SOCKET socket, Bool enable);
    static Int32                SetRecvBufferSize(SOCKET socket, Int32 size);
    static Int32                SetSendBufferSize(SOCKET socket, Int32 size);
    static Int32                SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);

    static Int32                BindAddress(SOCKET socket, const NetAddress& address);
    static Int32                BindAnyAddress(SOCKET socket, UInt16 port);
    static Int32                Listen(SOCKET socket, Int32 backlog = SOMAXCONN);
    static Int32                CloseSocket(SOCKET& socket);

private:
    static LPFN_CONNECTEX       sConnectEx;
    static LPFN_DISCONNECTEX    sDisconnectEx;
    static LPFN_ACCEPTEX        sAcceptEx;
};

template<typename T>
static Int32 SetSocketOpt(SOCKET socket, Int32 level, Int32 optName, const T& optVal)
{
    if (SOCKET_ERROR == ::setsockopt(socket, level, optName, reinterpret_cast<const char*>(&optVal), sizeof(optVal)))
    {
        return ::WSAGetLastError();
    }

    return 0;
}
