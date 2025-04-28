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

struct Buff
{
    Int64       id;
    Float32     remainTime;
};

// TEMP: Test 패킷
struct TestPacket
{
    Int64           id;
    Int32           hp;
    Int16           attack;
    Vector<Buff>    buffs;
};

void ServerPacketHandler::HandleTestPacket(Byte* packet, Int64 size)
{
    BufferReader reader(packet, size);
    PacketHeader header = {};
    reader >> header;

    Int64 id = 0;
    Int32 hp = 0;
    Int16 attack = 0;
    Int16 numBuffs = 0;
    reader >> id >> hp >> attack >> numBuffs;

    gLogger->Debug(TEXT_8("Packet: id={}, hp={}, attck={}, numBuffs={}"), id, hp, attack, numBuffs);

    Vector<Buff> buffs(numBuffs);
    buffs.resize(numBuffs);
    for (Int16 i = 0; i < numBuffs; ++i)
    {
        reader >> buffs[i].id >> buffs[i].remainTime;
        gLogger->Debug(TEXT_8("Buffs[{}]: id={}, remainTime={}"), i, buffs[i].id, buffs[i].remainTime);
    }
}
