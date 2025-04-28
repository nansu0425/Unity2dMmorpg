/*    GameServer/Network/Packet.h    */

#pragma once

enum ServerPacketId : Int32
{
    ServerPacketId_Test = 0x0001,
};

class ClientPacketHandler
{
public:
    static void     HandlePacket(Byte* packet, Int64 size);
};

class ServerPacketGenerator
{
public:
    static SharedPtr<SendBuffer>    GenerateTestPacket(Int64 id, Int32 hp, Int16 attack);
};
