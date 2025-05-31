/*    Protocol/Packet/Utils.h    */

#pragma once

#include "Protocol/Packet/Type.h"
#include "Core/Network/Session.h"

namespace proto
{
    class PacketUtils
    {
    public:     // payload 타입별로 Send 함수를 오버로딩
        {%- for proto_file, packets in proto_parser.packet_dict.items() %}
        {%- for packet in packets %}
        static void Send(const SharedPtr<core::Session>& target, const {{ packet.payload_type }}& payload) { Send(target, payload, PacketId::{{ packet.payload_type }}); }
        {%- endfor %}
        {%- endfor %}

    public:
        template<typename TPayload>
        static void Broadcast(const Vector<SharedPtr<core::Session>>& targets, const TPayload& payload)
        {
            for (const auto& target : targets)
            {
                Send(target, payload);
            }
        }

        // 전송할 Packet을 SendBuffer로 생성
        template<typename TPayload>
        static SharedPtr<core::SendBuffer> MakeSendBuffer(const TPayload& payload, PacketId id)
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

            return buffer;
        }

    private:
        template<typename TPayload>
        static void Send(const SharedPtr<core::Session>& target, const TPayload& payload, PacketId id)
        {
            target->SendAsync(MakeSendBuffer(payload, id));
        }
    };
} // namespace proto
