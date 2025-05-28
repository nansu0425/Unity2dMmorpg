/*    Protocol/Packet/Dispatcher.cpp    */

#include "Protocol/Pch.h"
#include "Protocol/Packet/Dispatcher.h"
#include "Core/Network/Session.h"

using namespace core;

namespace proto
{
    Int64 PacketDispatcher::DispatchReceivedPackets(SharedPtr<Session> owner, const Byte* buffer, Int64 numBytes)
    {
        Int64 packetOffset = 0;

        // 전달 가능한 모든 패킷을 핸들러에게 전달
        while (packetOffset < numBytes)
        {
            const Int64 remainingBytes = numBytes - packetOffset;

            // 패킷 헤더 크기만큼 수신했는지 확인
            if (remainingBytes < sizeof_64(PacketHeader))
            {
                break;
            }

            const PacketHeader* header = reinterpret_cast<const PacketHeader*>(buffer + packetOffset);
            ASSERT_CRASH_DEBUG(header->size > 0, "INVALID_PACKET_SIZE");

            // 패킷의 일부만 수신한 경우
            if (header->size > remainingBytes)
            {
                break;
            }

            // 패킷 id에 해당하는 핸들러에게 전달하여 패킷 처리
            Bool result = DispatchPacket(PacketView(owner, reinterpret_cast<const Byte*>(header)));
            ASSERT_CRASH_DEBUG(result, "DISPATCH_PACKET_FAILED");
            packetOffset += header->size;
        }

        return packetOffset;
    }

    PacketDispatcher::PacketDispatcher()
    {
        // Invalid 핸들러로 초기화
        for (Int32 i = 0; i < std::numeric_limits<Int16>::max() + 1; ++i)
        {
            mIdToHandler[i] = &PacketDispatcher::Handle_Invalid;
        }
    }

    Bool PacketDispatcher::Handle_Invalid(const PacketView& packet)
    {
        gLogger->Error(TEXT_8("Session[{}]: Invalid packet id: {}"), packet.GetOwner()->GetId(), packet.GetId());
        return false;
    }
} // namespace protocol
