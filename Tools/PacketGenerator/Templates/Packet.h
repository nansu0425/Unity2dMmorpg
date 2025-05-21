/*    {{ project_name }}/Network/Protocol/Packet.h    */

#pragma once

{%- for prefix, payloads in parser.payload_dict.items() %}
#include "{{ project_name }}/Network/Protocol/{{ prefix }}_Payload.pb.h"
{%- endfor %}

class Session;

enum class PacketId : Int16
{
    Invalid = 0,
    {%- for prefix, payloads in parser.payload_dict.items() %}
    {%- for payload in payloads %}
    {{ payload.name }} = {{ payload.pkt_id }},
    {%- endfor %}
    {%- endfor %}
};

#pragma pack(push, 1)
struct PacketHeader
{
    Int16       size = 0; // 헤더까지 포함한 패킷의 전체 크기
    PacketId    id = PacketId::Invalid; // 패킷 ID
};
#pragma pack(pop)

class Packet
{
public:
    explicit Packet(SharedPtr<Session> owner, const Byte* buffer)
        : mOwner(std::move(owner))
        , mHeader(reinterpret_cast<const PacketHeader*>(buffer))
        , mPayload(buffer + sizeof_16(PacketHeader))
    {}

    SharedPtr<Session>      GetOwner() const { return mOwner; }
    const PacketHeader*     GetHeader() const { return mHeader; }
    Int16                   GetSize() const { return mHeader->size; }
    Int16                   GetId() const { return static_cast<Int16>(mHeader->id); }
    const Byte*             GetPayload() const { return mPayload; }

private:
    SharedPtr<Session>      mOwner;
    const PacketHeader*     mHeader;
    const Byte*             mPayload;
};

class PacketHandlerMap
{
public:
    using PacketHandler     = Function<Bool(const Packet&)>;

public:
    Bool                HandlePacket(const Packet& packet) { return mIdToHandler[packet.GetId()](packet); }

protected:
    PacketHandlerMap();

    void                RegisterHandler(PacketHandler handler, PacketId id) { mIdToHandler[static_cast<Int16>(id)] = std::move(handler); }
    virtual void        RegisterAllHandlers() = 0;

    template<typename TPayload, typename TPayloadHandler>
    Bool                HandlePayload(TPayloadHandler handler, const Packet& packet)
    {
        TPayload payload;
        if (!payload.ParseFromArray(packet.GetPayload(), packet.GetSize() - sizeof_16(PacketHeader)))
        {
            return false;
        }

        // 페이로드 처리
        return handler(packet.GetOwner(), payload);
    }

    static Bool         Handle_Invalid(const Packet& packet);

private:
    PacketHandler       mIdToHandler[std::numeric_limits<Int16>::max() + 1];
};

class PacketUtils
{
public:
    template<typename TPayload>
    static SharedPtr<SendBuffer>    MakePacketBuffer(const TPayload& payload, PacketId packetId)
    {
        const Int16 payloadSize = static_cast<Int16>(payload.ByteSizeLong());
        const Int16 packetSize = sizeof_16(PacketHeader) + payloadSize;
        SharedPtr<SendBuffer> sendBuf = gSendChunkPool->Alloc(packetSize);

        // 헤더 설정
        PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuf->GetBuffer());
        header->size = packetSize;
        header->id = packetId;

        // 페이로드 직렬화
        ASSERT_CRASH(payload.SerializeToArray(header + 1, payloadSize), "SERIALIZE_TO_ARRAY_FAILED");
        sendBuf->OnWritten(packetSize);

        return sendBuf;
    }

    static Int64                    ProcessPackets(PacketHandlerMap& handlers, SharedPtr<Session> owner, const Byte* buffer, Int64 numBytes);
};
