/*    ServerEngine/Network/NetAddress.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Network/NetAddress.h"

NetAddress::NetAddress(SOCKADDR_IN address)
    : mAddress(address)
{}

NetAddress::NetAddress(String16View ip, UInt16 port)
{
    ::memset(&mAddress, 0, sizeof(mAddress));
    mAddress.sin_family = AF_INET;
    mAddress.sin_addr = Ip2Addr(ip);
    mAddress.sin_port = ::htons(port);
}

String16 NetAddress::GetIp() const
{
    Char16 buffer[100] = {};
    ::InetNtop(AF_INET, &mAddress.sin_addr, OUT buffer, NUM_ELEM_64(buffer));

    return String16(buffer);
}

IN_ADDR NetAddress::Ip2Addr(String16View ip)
{
    IN_ADDR addr;
    ::InetPton(AF_INET, ip.data(), OUT &addr);

    return addr;
}
