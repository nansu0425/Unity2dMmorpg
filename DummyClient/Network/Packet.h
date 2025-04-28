/*    DummyClient/Network/Packet..h    */

#pragma once

enum ServerPacketId : Int32
{
    ServerPacketId_Test = 0x0001,
};

class ServerPacketHandler
{
public:
    static void HandlePacket(Byte* packet, Int64 size);

private:
    static void HandleTestPacket(Byte* packet, Int64 size);
};
