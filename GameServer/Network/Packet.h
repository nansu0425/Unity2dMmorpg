/*    GameServer/Network/Packet.h    */

#pragma once

#include "Packet/Generated/S_Test_generated.h"

enum class PacketId : Int32
{
    S_Test = 0x0000'0001,
};

class ClientPacketHandler
{
public:
    static void     HandlePacket(Byte* packet, Int64 size);
};

class ServerPacketGenerator
{
public:
    static SharedPtr<SendBuffer> MakeSendBuffer(const flatbuffers::FlatBufferBuilder& fbb, PacketId packetId);
};
