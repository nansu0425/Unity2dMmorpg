/*    GameServer/Network/Packet.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/Packet.h"
#include "ServerEngine/Network/Session.h"

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

SharedPtr<SendBuffer> ServerPacketGenerator::GenerateTestPacket(Int64 id, Int32 hp, Int16 attack, Vector<Buff> buffs)
{
    // 패킷 생성
    SharedPtr<SendBuffer> sendBuffer = gSendBufferManager->Open(4096);
    BufferWriter writer(sendBuffer->GetBuffer(), sendBuffer->GetAllocSize());
    PacketHeader* header = writer.Reserve<PacketHeader>();
    writer << id << hp << attack << static_cast<Int16>((buffs.size()));
    for (auto [id, remainTime] : buffs)
    {
        writer << id << remainTime;
    }

    // 패킷 헤더 설정
    header->size = static_cast<Int32>(writer.GetWrittenSize());
    header->id = ServerPacketId_Test;
    sendBuffer->Close(writer.GetWrittenSize());

    return sendBuffer;
}
