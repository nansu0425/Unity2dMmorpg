/*    DummyClient/Main.cpp    */

#include "DummyClient/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Network/Socket.h"
#include "ServerEngine/Network/Address.h"

void ClientThread()
{
    Int64 result = SUCCESS;

    // 1. 소켓 생성
    SOCKET clientSocket = INVALID_SOCKET;
    if (result = SocketUtils::CreateSocket(clientSocket))
    {
        std::cerr << "Socket creation failed: " << result << std::endl;
        return;
    }

    // 2. 서버 주소 설정
    NetAddress address(TEXT_16("127.0.0.1"), 7777);

    // 3. 서버에 연결
    if (SOCKET_ERROR == ::connect(clientSocket, reinterpret_cast<const sockaddr*>(&address.GetAddress()), SIZE_32(sockaddr_in)))
    {
        result = ::WSAGetLastError();
        std::cerr << "Connection failed: " << result << std::endl;
        SocketUtils::CloseSocket(clientSocket);
        return;
    }

    std::cout << "Connected to 127.0.0.1:7777" << std::endl;

    // 4. 1초 마다 데이터 전송 및 수신
    while (true)
    {
        Char8 sendBuffer[32] = "Echo Test";

        // 데이터 전송
        Int64 bytesSent = ::send(clientSocket, sendBuffer, SIZE_32(sendBuffer), 0);
        if (SOCKET_ERROR == bytesSent)
        {
            result = ::WSAGetLastError();
            std::cerr << "Send failed: " << result << std::endl;
            break;
        }

        // 데이터 수신
        Char8 recvBuffer[32] = {};
        Int64 bytesReceived = ::recv(clientSocket, recvBuffer, SIZE_32(recvBuffer), 0);
        if (SOCKET_ERROR == bytesReceived)
        {
            result = ::WSAGetLastError();
            std::cerr << "Receive failed: " << result << std::endl;
            break;
        }
        std::cout << "Received: " << recvBuffer << std::endl;

        Sleep(1000);
    }

    SocketUtils::CloseSocket(clientSocket);
}

int main()
{
    mi_version();

    for (Int64 i = 0; i < 10; ++i)
    {
        gThreadManager->Launch(ClientThread);
    }
    gThreadManager->Join();

    return 0;
}
