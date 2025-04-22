/*    ServerEngine/Network/Socket.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Network/Socket.h"
#include "ServerEngine/Network/Address.h"
#include "ServerEngine/Network/Session.h"
#include "ServerEngine/Io/Event.h"

void SocketUtils::Init()
{
    WSADATA wsaData;
    ASSERT_CRASH(SUCCESS == ::WSAStartup(MAKEWORD(2, 2), OUT &wsaData), "WSA_STARTUP_FAILED");
    // 런타임에 주소를 가져온다
    SOCKET dummySocket = INVALID_SOCKET;
    CreateSocket(OUT dummySocket);
    ASSERT_CRASH(SUCCESS == BindWindowsFunction(dummySocket, WSAID_CONNECTEX, OUT reinterpret_cast<LPVOID*>(&sConnectEx)), "CONNECTEX_BIND_FAILED");
    ASSERT_CRASH(SUCCESS == BindWindowsFunction(dummySocket, WSAID_DISCONNECTEX, OUT reinterpret_cast<LPVOID*>(&sDisconnectEx)), "DISCONNECTEX_BIND_FAILED");
    ASSERT_CRASH(SUCCESS == BindWindowsFunction(dummySocket, WSAID_ACCEPTEX, OUT reinterpret_cast<LPVOID*>(&sAcceptEx)), "ACCEPTEX_BIND_FAILED");
    CloseSocket(OUT dummySocket);
}

void SocketUtils::Cleanup()
{
    ::WSACleanup();
}

Int64 SocketUtils::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* function)
{
    DWORD bytesReturned = 0;
    if (SOCKET_ERROR == ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), function, sizeof(*function), OUT &bytesReturned, nullptr, nullptr))
    {
        return ::WSAGetLastError();
    }

    return SUCCESS;
}

Int64 SocketUtils::CreateSocket(SOCKET& socket)
{
    socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == socket)
    {
        return ::WSAGetLastError();
    }

    return SUCCESS;
}

Int64 SocketUtils::SetLinger(SOCKET socket, UInt16 onOff, UInt16 lingerTime)
{
    LINGER linger = {};
    linger.l_onoff = onOff;
    linger.l_linger = lingerTime;

    return SetSocketOpt(socket, SOL_SOCKET, SO_LINGER, linger);
}

Int64 SocketUtils::SetReuseAddress(SOCKET socket, Bool enable)
{
    return SetSocketOpt(socket, SOL_SOCKET, SO_REUSEADDR, enable);
}

Int64 SocketUtils::SetNoDelay(SOCKET socket, Bool enable)
{
    return SetSocketOpt(socket, SOL_SOCKET, TCP_NODELAY, enable);
}

Int64 SocketUtils::SetRecvBufferSize(SOCKET socket, Int64 size)
{
    return SetSocketOpt(socket, SOL_SOCKET, SO_RCVBUF, size);
}

Int64 SocketUtils::SetSendBufferSize(SOCKET socket, Int64 size)
{
    return SetSocketOpt(socket, SOL_SOCKET, SO_SNDBUF, size);
}

Int64 SocketUtils::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket)
{
    return SetSocketOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
}

Int64 SocketUtils::BindAddress(SOCKET socket, const NetAddress& address)
{
    if (SOCKET_ERROR == ::bind(socket, reinterpret_cast<const SOCKADDR*>(&address.GetAddress()), sizeof(SOCKADDR_IN)))
    {
        return ::WSAGetLastError();
    }

    return SUCCESS;
}

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

Int64 SocketUtils::Listen(SOCKET socket, Int32 backlog)
{
    if (SOCKET_ERROR == ::listen(socket, backlog))
    {
        return ::WSAGetLastError();
    }

    return SUCCESS;
}

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

Int64 SocketUtils::AcceptAsync(SOCKET listenSocket, Session* session, Int64& numBytes, AcceptEvent* event)
{
    if (FALSE == sAcceptEx(listenSocket, session->GetSocket(), session->mRecvBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, reinterpret_cast<LPDWORD>(&numBytes), static_cast<LPOVERLAPPED>(event)))
    {
        return ::WSAGetLastError();
    }

    return SUCCESS;
}

LPFN_CONNECTEX           SocketUtils::sConnectEx = nullptr;
LPFN_DISCONNECTEX        SocketUtils::sDisconnectEx = nullptr;
LPFN_ACCEPTEX            SocketUtils::sAcceptEx = nullptr;
