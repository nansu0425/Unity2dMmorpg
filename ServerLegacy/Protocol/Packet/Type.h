/*    Protocol/Packet/Type.h    */

#pragma once

#include "Protocol/Packet/Id.h"

namespace core
{
    class Session;
} // namespace core

namespace proto
{
    struct RawPacket
    {
        SharedPtr<core::Session> owner; // 패킷 소유자 세션
        Vector<Byte> data; // 패킷 데이터
    };

#pragma pack(push, 1)
    struct PacketHeader
    {
        Int16       size = 0; // 헤더까지 포함한 패킷의 전체 크기
        PacketId    id = PacketId::Invalid;
    };
#pragma pack(pop)

    // 직렬화된 바이너리 패킷 데이터를 쉽게 읽을 수 있도록 뷰를 제공
    class PacketView
    {
    public:
        explicit PacketView(const SharedPtr<core::Session>& owner, const Byte* buffer)
            : mOwner(owner)
            , mHeader(reinterpret_cast<const PacketHeader*>(buffer))
            , mPayload(buffer + sizeof_16(PacketHeader))
        {}

        const SharedPtr<core::Session>&     GetOwner() const { return mOwner; }
        const PacketHeader*                 GetHeader() const { return mHeader; }
        Int16                               GetSize() const { return mHeader->size; }
        Int16                               GetId() const { return static_cast_16(mHeader->id); }
        const Byte*                         GetPayload() const { return mPayload; }

    private:
        const SharedPtr<core::Session>&     mOwner;
        const PacketHeader*                 mHeader;
        const Byte*                         mPayload;
    };
} // namespace proto
