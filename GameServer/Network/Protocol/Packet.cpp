/*    GameServer/Network/Protocol/Packet.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/Protocol/Packet.h"
#include "GameServer/Network/Session.h"

PacketHandlerMap::PacketHandlerMap()
{
    // Invalid 핸들러로 초기화
    for (Int32 i = 0; i < std::numeric_limits<Int16>::max() + 1; ++i)
    {
        mIdToHandler[i] = [this](SharedPtr<Session> session, const Byte* buffer, Int64 numBytes)
            {
                return Handle_Invalid(std::move(session), buffer, numBytes);
            };
    }
}

Bool PacketHandlerMap::HandlePacket(SharedPtr<Session> session, const Byte* buffer, Int64 numBytes)
{
    const PacketHeader* header = reinterpret_cast<const PacketHeader*>(buffer);
    // 패킷 id에 매핑된 핸들러 호출
    return mIdToHandler[static_cast<Int16>(header->id)](std::move(session), buffer, numBytes);
}

Bool PacketHandlerMap::Handle_Invalid(SharedPtr<Session> session, const Byte* buffer, Int64 numBytes)
{
    gLogger->Error("Session[{}]: Invalid packet", session->GetId());
    return false;
}

Int64 PacketUtils::ProcessPackets(PacketHandlerMap& handlers, SharedPtr<Session> session, const Byte* buffer, Int64 numBytes)
{
    Int64 processedSize = 0;

    // 처리 가능한 모든 패킷 처리
    while (processedSize < numBytes)
    {
        // 패킷 헤더를 읽어 패킷 크기와 ID를 확인
        const PacketHeader* header = reinterpret_cast<const PacketHeader*>(buffer + processedSize);
        ASSERT_CRASH(header->size > 0, "INVALID_PACKET_SIZE");

        // 패킷의 일부만 수신한 경우
        if (header->size > numBytes - processedSize)
        {
            break;
        }

        // 패킷 처리
        handlers.HandlePacket(session, buffer + processedSize, header->size);
        processedSize += header->size;
    }

    return processedSize;
}
