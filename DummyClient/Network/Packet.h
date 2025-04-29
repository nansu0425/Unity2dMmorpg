/*    DummyClient/Network/Packet..h    */

#pragma once

enum class PacketId : Int32
{
    S_Test = 0x0000'0001,
};

class ServerPacketHandler
{
public:
    static void HandlePacket(Byte* packet, Int64 size);

private:
    static void HandleTestPacket(Byte* packet, Int64 size);
};
