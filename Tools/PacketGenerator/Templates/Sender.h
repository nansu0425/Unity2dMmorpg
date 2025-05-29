/*    Protocol/Packet/Sender.h    */

#pragma once

#include "Core/Network/Session.h"
#include "Protocol/Packet/Dispatcher.h"

namespace proto
{
    class PacketSender
    {
    public:
        {%- for proto_file, packets in proto_parser.packet_dict.items() %}
        {%- for packet in packets %}
        static void Send(SharedPtr<core::Session> target, const {{ packet.payload_type }}& payload)
        {
            Send(std::move(target), payload, PacketId::{{ packet.payload_type }});
        }
        {% endfor %}
        {%- endfor %}
        template<typename TPayload>
        static void Broadcast(Vector<SharedPtr<core::Session>>& targets, const TPayload& payload)
        {
            for (auto& target : targets)
            {
                Send(std::move(target), payload);
            }
        }

    private:
        template<typename TPayload>
        static void Send(SharedPtr<core::Session> target, const TPayload& payload, PacketId id)
        {
            using namespace core;

            const Int16 payloadSize = static_cast_16(payload.ByteSizeLong());
            const Int16 packetSize = sizeof_16(PacketHeader) + payloadSize;
            SharedPtr<SendBuffer> buffer = gSendChunkPool->Alloc(packetSize);

            // 헤더 설정
            PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer->GetBuffer());
            header->size = packetSize;
            header->id = id;

            // 페이로드 직렬화
            ASSERT_CRASH(payload.SerializeToArray(header + 1, payloadSize), "SERIALIZE_TO_ARRAY_FAILED");
            buffer->OnWritten(packetSize);

            // 세션에 전송
            target->SendAsync(std::move(buffer));
        }
    };
} // namespace proto
