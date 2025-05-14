/*    GameServer/Network/Protocol/Packet.h    */

#pragma once
#include "GameServer/Network/Protocol/C2S_Payload.pb.h"
#include "GameServer/Network/Protocol/S2C_Payload.pb.h"

class Session;

enum class PacketId : Int16
{
    Invalid = 0,
    C2S_EnterRoom = 1000,
    C2S_Chat = 1001,
    S2C_EnterRoom = 1002,
    S2C_Chat = 1003,
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
        , mPayload(buffer + SIZE_16(PacketHeader))
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
    using Handler       = Function<Bool(const Packet&)>;

public:
    Bool                HandlePacket(const Packet& packet) { return mIdToHandler[packet.GetId()](packet); }

protected:
    PacketHandlerMap();

    void                RegisterHandler(Handler handler, PacketId id) { mIdToHandler[static_cast<Int16>(id)] = std::move(handler); }
    virtual void        RegisterAllHandlers() = 0;

    template<typename TPayload, typename THandler>
    Bool                HandlePayload(THandler handler, const Packet& packet)
    {
        TPayload payload;
        if (!payload.ParseFromArray(packet.GetPayload(), packet.GetSize() - SIZE_16(PacketHeader)))
        {
            return false;
        }

        // 페이로드 처리
        return handler(packet.GetOwner(), payload);
    }

    static Bool         Handle_Invalid(const Packet& packet);

private:
    Handler             mIdToHandler[std::numeric_limits<Int16>::max() + 1];
};

class PacketUtils
{
public:
    template<typename TPayload>
    static SharedPtr<SendBuffer>    MakePacketBuffer(const TPayload& payload, PacketId packetId)
    {
        const Int16 payloadSize = static_cast<Int16>(payload.ByteSizeLong());
        const Int16 packetSize = SIZE_16(PacketHeader) + payloadSize;
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