/*    DummyClient/Network/Packet.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Network/Packet.h"
#include "ServerEngine/Network/Session.h"
#include "Packet/Generated/S_Test_generated.h"

void ServerPacketHandler::HandlePacket(Byte* packet, Int64 size)
{
    BufferReader reader(packet, size);
    PacketHeader header = {};
    reader >> header;

    switch (static_cast<PacketId>(header.id))
    {
    case PacketId::S_Test:
        HandleTestPacket(packet, size);
        break;
    default:
        break;
    }
}

void ServerPacketHandler::HandleTestPacket(Byte* packet, Int64 size)
{
    Byte* data = packet + SIZE_32(PacketHeader);

    const Packet::S_Test* test = Packet::GetS_Test(data);
    gLogger->Debug(TEXT_16("Test Packet: id={}, hp={}, attack={}"), test->id(), test->hp(), test->attack());

    for (UInt32 i = 0; i < test->buffs()->size(); ++i)
    {
        gLogger->Debug(TEXT_16("Buff {}: id={}, remain_time={}")
                       , i
                       , test->buffs()->Get(i)->id()
                       , test->buffs()->Get(i)->remain_time());
        for (UInt32 j = 0; j < test->buffs()->Get(i)->victims()->size(); ++j)
        {
            gLogger->Debug(TEXT_16("Victim {}: {}"), j, test->buffs()->Get(i)->victims()->Get(j));
        }
    }
}
