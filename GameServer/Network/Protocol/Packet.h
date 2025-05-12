/*    GameServer/Network/Protocol/Packet.h    */

#pragma once

#include "GameServer/Network/Protocol/ServerPayload.pb.h"
#include "GameServer/Network/Protocol/ClientPayload.pb.h"

class Session;

enum class PacketId : Int16
{
    Invalid = 0,
    // 서버 패킷
    EnterRoomResponse = 1000,
    ChatBroadcast,
    // 클라이언트 패킷
    EnterRoomRequest = 2000,
    ChatNotify,
};

#pragma pack(push, 1)
struct PacketHeader
{
    Int16       size = 0;   // 헤더까지 포함한 패킷의 전체 크기
    PacketId    id = PacketId::Invalid; // 패킷 ID
};
#pragma pack(pop)

class PacketHandlerMap
{
public:
    using Handler       = Function<Bool(SharedPtr<Session>, const Byte*, Int64)>;

public:
    Bool                HandlePacket(SharedPtr<Session> session, const Byte* buffer, Int64 numBytes);

protected:
    PacketHandlerMap();

    void                RegisterHandler(Handler handler, PacketId id) { mIdToHandler[static_cast<Int16>(id)] = std::move(handler); }
    virtual void        RegisterAllHandlers() = 0;

    template<typename TPayload, typename THandler>
    Bool                HandlePayload(THandler handler, SharedPtr<Session> session, const Byte* buffer, Int64 numBytes)
    {
        TPayload payload;
        if (!payload.ParseFromArray(buffer + SIZE_16(PacketHeader), static_cast<Int32>(numBytes) - SIZE_16(PacketHeader)))
        {
            return false;
        }

        // 페이로드 처리
        return handler(std::move(session), payload);
    }

    static Bool         Handle_Invalid(SharedPtr<Session> session, const Byte* buffer, Int64 numBytes);

private:
    Handler             mIdToHandler[std::numeric_limits<Int16>::max() + 1];
};

class PacketUtils
{
public:
    template<typename T>
    static SharedPtr<SendBuffer>    MakeSendBuffer(const T& payload, PacketId packetId)
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

    static Int64                    ProcessPackets(PacketHandlerMap& handlers, SharedPtr<Session> session, const Byte* buffer, Int64 numBytes);
};
