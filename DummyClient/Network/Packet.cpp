/*    DummyClient/Network/Packet.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Network/Packet.h"
#include "ServerEngine/Network/Session.h"

void ServerPacketHandler::HandlePacket(Byte* packet, Int64 size)
{
    BufferReader reader(packet, size);
    PacketHeader header = {};
    reader >> header;

    switch (header.id)
    {
    case ServerPacketId_Test:
        HandleTestPacket(packet, size);
        break;
    default:
        break;
    }
}

// TEMP: Test 패킷
struct TestPacket
{
    Int64 id;
    Int32 hp;
    Int16 attack;
};

void ServerPacketHandler::HandleTestPacket(Byte* packet, Int64 size)
{
    BufferReader reader(packet, size);
    PacketHeader header = {};
    reader >> header;

    Int64 id = 0;
    Int32 hp = 0;
    Int16 attack = 0;
    reader >> id >> hp >> attack;

    gLogger->Debug(TEXT_8("Packet: id={}, hp={}, attck={}"), id, hp, attack);
}
