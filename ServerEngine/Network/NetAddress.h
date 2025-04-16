/*    ServerEngine/Network/NetAddress.h    */

#pragma once

class NetAddress
{
public:
    NetAddress() = default;
    NetAddress(SOCKADDR_IN address);
    NetAddress(String16View ip, UInt16 port);

    const SOCKADDR_IN&              GetAddress() const  { return mAddress; }
    MemoryPoolAllocator::String16   GetIp() const;
    UInt16                          GetPort() const     { return ::ntohs(mAddress.sin_port); }

public:
    static IN_ADDR  Ip2Addr(String16View ip);

private:
    SOCKADDR_IN mAddress = {};
};
