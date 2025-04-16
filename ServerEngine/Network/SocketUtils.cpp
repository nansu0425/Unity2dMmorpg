/*    ServerEngine/Network/SocketUtils.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Network/SocketUtils.h"
#include "ServerEngine/Network/NetAddress.h"

void SocketUtils::Init()
{
    WSADATA wsaData;
    ASSERT_CRASH(0 == ::WSAStartup(MAKEWORD(2, 2), OUT &wsaData), "WSA_STARTUP_FAILED");
    // 런타임에 주소를 가져온다
    SOCKET dummySocket = INVALID_SOCKET;
    CreateSocket(OUT dummySocket);
    ASSERT_CRASH(0 == BindWindowsFunction(dummySocket, WSAID_CONNECTEX, OUT reinterpret_cast<LPVOID*>(&sConnectEx)), "CONNECTEX_BIND_FAILED");
    ASSERT_CRASH(0 == BindWindowsFunction(dummySocket, WSAID_DISCONNECTEX, OUT reinterpret_cast<LPVOID*>(&sDisconnectEx)), "DISCONNECTEX_BIND_FAILED");
    ASSERT_CRASH(0 == BindWindowsFunction(dummySocket, WSAID_ACCEPTEX, OUT reinterpret_cast<LPVOID*>(&sAcceptEx)), "ACCEPTEX_BIND_FAILED");
    CloseSocket(OUT dummySocket);
}

void SocketUtils::Cleanup()
{
    ::WSACleanup();
}

Int32 SocketUtils::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* function)
{
    DWORD bytesReturned = 0;
    if (SOCKET_ERROR == ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), function, sizeof(*function), OUT &bytesReturned, nullptr, nullptr))
    {
        return ::WSAGetLastError();
    }

    return 0;
}

Int32 SocketUtils::CreateSocket(SOCKET& socket)
{
    socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == socket)
    {
        return ::WSAGetLastError();
    }

    return 0;
}

Int32 SocketUtils::SetLinger(SOCKET socket, UInt16 onOff, UInt16 lingerTime)
{
    LINGER linger = {};
    linger.l_onoff = onOff;
    linger.l_linger = lingerTime;

    return SetSocketOpt(socket, SOL_SOCKET, SO_LINGER, linger);
}

Int32 SocketUtils::SetReuseAddress(SOCKET socket, Bool enable)
{
    return SetSocketOpt(socket, SOL_SOCKET, SO_REUSEADDR, enable);
}

Int32 SocketUtils::SetNoDelay(SOCKET socket, Bool enable)
{
    return SetSocketOpt(socket, SOL_SOCKET, TCP_NODELAY, enable);
}

Int32 SocketUtils::SetRecvBufferSize(SOCKET socket, Int32 size)
{
    return SetSocketOpt(socket, SOL_SOCKET, SO_RCVBUF, size);
}

Int32 SocketUtils::SetSendBufferSize(SOCKET socket, Int32 size)
{
    return SetSocketOpt(socket, SOL_SOCKET, SO_SNDBUF, size);
}

Int32 SocketUtils::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket)
{
    return SetSocketOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
}

Int32 SocketUtils::BindAddress(SOCKET socket, const NetAddress& address)
{
    if (SOCKET_ERROR == ::bind(socket, reinterpret_cast<const SOCKADDR*>(&address.GetAddress()), sizeof(SOCKADDR_IN)))
    {
        return ::WSAGetLastError();
    }

    return 0;
}

Int32 SocketUtils::BindAnyAddress(SOCKET socket, UInt16 port)
{
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = ::htons(port);

    if (SOCKET_ERROR == ::bind(socket, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)))
    {
        return ::WSAGetLastError();
    }

    return 0;
}

Int32 SocketUtils::Listen(SOCKET socket, Int32 backlog)
{
    if (SOCKET_ERROR == ::listen(socket, backlog))
    {
        return ::WSAGetLastError();
    }

    return 0;
}

Int32 SocketUtils::CloseSocket(SOCKET& socket)
{ 
    if (INVALID_SOCKET == socket)
    {
        return 0;
    }

    if (SOCKET_ERROR == ::closesocket(socket))
    {
        return ::WSAGetLastError();
    }
    socket = INVALID_SOCKET;

    return 0;
}

LPFN_CONNECTEX           SocketUtils::sConnectEx = nullptr;
LPFN_DISCONNECTEX        SocketUtils::sDisconnectEx = nullptr;
LPFN_ACCEPTEX            SocketUtils::sAcceptEx = nullptr;
