/*    GameServer/Network/Packet.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/Packet.h"
#include "ServerEngine/Network/Session.h"
#include "Packet/Generated/S_Test_generated.h"

void ClientPacketHandler::HandlePacket(Byte* packet, Int64 size)
{
    BufferReader reader(packet, size);
    PacketHeader header = {};
    reader.Peek(&header);

    // TODO: id에 따라 패킷 처리
    /*switch (header.id)
    {
    default:
        break;
    }*/
}

SharedPtr<SendBuffer> ServerPacketGenerator::MakeSendBuffer(const flatbuffers::FlatBufferBuilder& fbb, PacketId packetId)
{
    const Int32 dataSize = static_cast<Int32>(fbb.GetSize());
    const Int32 packetSize = dataSize + SIZE_32(PacketHeader);

    SharedPtr<SendBuffer> buffer = gSendBufferManager->Open(packetSize);
    // 헤더 설정
    PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer->GetBuffer());
    header->size = packetSize;
    header->id = static_cast<Int32>(packetId);
    // 데이터 복사
    ::memcpy(header + 1, fbb.GetBufferPointer(), dataSize);
    buffer->Close(packetSize);

    return buffer;
}
