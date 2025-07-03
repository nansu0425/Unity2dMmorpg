/*    Protocol/Packet/Queue.cpp    */

#include "Protocol/Pch.h"
#include "Protocol/Packet/Queue.h"
#include "Protocol/Packet/Type.h"

namespace proto
{
    void PacketQueue::Push(const SharedPtr<RawPacket>& packet)
    {
        while (!mQueue.enqueue(packet))
        {
            // 큐가 가득 찬 경우 대기합니다.
            core::gLogger->Warn(TEXT_8("PacketQueue is full, retrying to enqueue packet"));
            _mm_pause();
        }
    }

    Int64 PacketQueue::Push(const SharedPtr<core::Session>& owner, const Byte* buffer, Int64 numBytes)
    {
        Int64 packetOffset = 0;

        // 모든 패킷을 큐에 넣는다
        while (packetOffset < numBytes)
        {
            const Int64 remainingBytes = numBytes - packetOffset;

            // 패킷 헤더 크기만큼 있는지 확인
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

            // 패킷을 큐에 추가
            SharedPtr<RawPacket> packet = std::make_shared<RawPacket>(owner, buffer + packetOffset);
            Push(packet);

            // 다음 패킷 오프셋으로 이동
            packetOffset += header->size;
        }

        return packetOffset;
    }

    Bool PacketQueue::TryPop(SharedPtr<RawPacket>& packet)
    {
        return mQueue.try_dequeue(packet);
    }
}
