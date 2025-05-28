/*    Protocol/Packet/Utils.h    */

#pragma once

#include "Protocol/Packet/Dispatcher.h"

namespace proto::util
{
    template<typename TPayload>
    SharedPtr<core::SendBuffer>  MakeSendPacketBuffer(const TPayload& payload, PacketId packetId)
    {
        using namespace core;

        const Int16 payloadSize = static_cast_16(payload.ByteSizeLong());
        const Int16 packetSize = sizeof_16(PacketHeader) + payloadSize;
        SharedPtr<SendBuffer> buffer = gSendChunkPool->Alloc(packetSize);

        // 헤더 설정
        PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer->GetBuffer());
        header->size = packetSize;
        header->id = packetId;

        // 페이로드 직렬화
        ASSERT_CRASH(payload.SerializeToArray(header + 1, payloadSize), "SERIALIZE_TO_ARRAY_FAILED");
        buffer->OnWritten(packetSize);

        return buffer;
    }

    Int64    ProcessReceivedPackets(PacketDispatcher& dispatcher, SharedPtr<core::Session> owner, const Byte* buffer, Int64 numBytes);
} // namespace proto::util
