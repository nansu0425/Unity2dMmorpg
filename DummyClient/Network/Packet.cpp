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
    String16        name;
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
    gLogger->Debug(TEXT_16("Packet: id={}, hp={}, attck={}"), id, hp, attack);

    // 버프 읽기
    Int16 numBuffs = 0;
    reader >> numBuffs;
    Vector<Buff> buffs(numBuffs);
    buffs.resize(numBuffs);
    for (Int16 i = 0; i < numBuffs; ++i)
    {
        reader >> buffs[i].id >> buffs[i].remainTime;
        gLogger->Debug(TEXT_16("Buffs[{}]: id={}, remainTime={}"), i, buffs[i].id, buffs[i].remainTime);
    }

    // 이름 읽기
    Int16 nameSize = 0;
    reader >> nameSize;
    String16 name;
    name.resize(nameSize);
    reader.Read(reinterpret_cast<Byte*>(name.data()), nameSize * SIZE_64(Char16));
    gLogger->Debug(TEXT_16("Name: {}"), name);
}
