/*    GameServer/Network/Protocol/Packet.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/Protocol/Packet.h"
#include "GameServer/Network/Session.h"

PacketHandlerMap::PacketHandlerMap()
{
    // Invalid 핸들러로 초기화
    for (Int32 i = 0; i < std::numeric_limits<Int16>::max() + 1; ++i)
    {
        mIdToHandler[i] = [this](const Packet& packet) { return Handle_Invalid(packet); };
    }
}


Bool PacketHandlerMap::Handle_Invalid(const Packet& packet)
{
    gLogger->Error(TEXT_8("Session[{}]: Invalid packet id: {}"), packet.GetOwner()->GetId(), packet.GetId());
    return false;
}

Int64 PacketUtils::ProcessPackets(PacketHandlerMap& handlers, SharedPtr<Session> owner, const Byte* buffer, Int64 numBytes)
{
    Int64 processedSize = 0;

    // 처리 가능한 모든 패킷 처리
    while (processedSize < numBytes)
    {
        // 패킷 헤더 크기만큼 수신했는지 확인
        if (numBytes - processedSize < sizeof_64(PacketHeader))
        {
            break;
        }

        // 패킷 헤더를 읽어 패킷 크기와 ID를 확인
        const PacketHeader* header = reinterpret_cast<const PacketHeader*>(buffer + processedSize);
        ASSERT_CRASH_DEBUG(header->size > 0, "INVALID_PACKET_SIZE");

        // 패킷의 일부만 수신한 경우
        if (header->size > numBytes - processedSize)
        {
            break;
        }

        // 패킷 처리
        handlers.HandlePacket(Packet(owner, buffer + processedSize));
        processedSize += header->size;
    }

    return processedSize;
}