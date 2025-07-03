/*    Protocol/Packet/Type.h    */

#pragma once

#include "Protocol/Packet/Id.h"

namespace core
{
    class Session;
} // namespace core

namespace proto
{
#pragma pack(push, 1)
    struct PacketHeader
    {
        Int16       size = 0; // 헤더까지 포함한 패킷의 전체 크기
        PacketId    id = PacketId::Invalid;
    };
#pragma pack(pop)

    // 패킷을 직렬화된 바이너리 형태의 데이터로 소유
    class RawPacket
    {
    public:
        explicit RawPacket(const SharedPtr<core::Session>& owner, const Byte* packet)
            : mOwner(owner)
            , mData(packet, packet + reinterpret_cast<const PacketHeader*>(packet)->size)
        {}

        const SharedPtr<core::Session>& GetOwner() const { return mOwner; }

        const PacketHeader* GetHeader() const { return reinterpret_cast<const PacketHeader*>(mData.data()); }
        const Byte* GetPayload() const { return mData.data() + sizeof_16(PacketHeader); }

        Int16 GetSize() const { return GetHeader()->size; }
        Int16 GetId() const { return static_cast_16(GetHeader()->id); }

    private:
        SharedPtr<core::Session> mOwner; // 패킷 소유자 세션
        Vector<Byte> mData; // 패킷 데이터
    };
} // namespace proto
