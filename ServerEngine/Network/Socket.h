/*    ServerEngine/Network/Socket.h    */

#pragma once

class NetAddress;
class Session;
struct AcceptEvent;

/*
 * 소켓 처리 유틸리티 클래스
 * 함수가 반환하는 값이 SUCCESS가 아니면 에러 코드
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
    static Int64                SetRecvBufferSize(SOCKET socket, Int64 size);
    static Int64                SetSendBufferSize(SOCKET socket, Int64 size);
    static Int64                SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);

    static Int64                BindAddress(SOCKET socket, const NetAddress& address);
    static Int64                BindAnyAddress(SOCKET socket, UInt16 port);
    static Int64                Listen(SOCKET socket, Int32 backlog = SOMAXCONN);
    static Int64                CloseSocket(SOCKET& socket);

    static Int64                AcceptAsync(SOCKET listenSocket, Session* session, Int64& numBytes, AcceptEvent* event);

private:
    static LPFN_CONNECTEX       sConnectEx;
    static LPFN_DISCONNECTEX    sDisconnectEx;
    static LPFN_ACCEPTEX        sAcceptEx;
};

template<typename T>
static Int64 SetSocketOpt(SOCKET socket, Int32 level, Int32 optName, const T& optVal)
{
    if (SOCKET_ERROR == ::setsockopt(socket, level, optName, reinterpret_cast<const char*>(&optVal), SIZE_32(optVal)))
    {
        return ::WSAGetLastError();
    }

    return SUCCESS;
}
