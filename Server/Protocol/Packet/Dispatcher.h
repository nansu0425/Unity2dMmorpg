/*    Protocol/Packet/Dispatcher.h    */

#pragma once
#include "Protocol/Packet/C2S_Payload.pb.h"
#include "Protocol/Packet/S2C_Payload.pb.h"

namespace core
{
    class Session;
}

namespace proto
{
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
        PacketId    id = PacketId::Invalid;
    };
#pragma pack(pop)

    // 직렬화된 바이너리 패킷 데이터를 쉽게 읽을 수 있도록 뷰를 제공
    class PacketView
    {
    public:
        explicit PacketView(SharedPtr<core::Session> owner, const Byte* buffer)
            : mOwner(std::move(owner))
            , mHeader(reinterpret_cast<const PacketHeader*>(buffer))
            , mPayload(buffer + sizeof_16(PacketHeader))
        {}

        SharedPtr<core::Session>        GetOwner() const { return mOwner; }
        const PacketHeader*             GetHeader() const { return mHeader; }
        Int16                           GetSize() const { return mHeader->size; }
        Int16                           GetId() const { return static_cast_16(mHeader->id); }
        const Byte*                     GetPayload() const { return mPayload; }

    private:
        SharedPtr<core::Session>        mOwner;
        const PacketHeader*             mHeader;
        const Byte*                     mPayload;
    };

    // 패킷 ID를 기반으로 들어오는 패킷을 지정된 핸들러 함수에 전달
    class PacketDispatcher
    {
    public:
        Int64               DispatchReceivedPackets(SharedPtr<core::Session> owner, const Byte* buffer, Int64 numBytes);

    protected:
                            PacketDispatcher();
        // PacketDispatcher를 상속받은 클래스에서 RegisterHandler()로 모든 패킷 핸들러 등록
        virtual void        RegisterAllHandlers() = 0;

        template<typename TPayload, typename TPayloadHandler>
        void                RegisterHandler(TPayloadHandler handler, PacketId id)
        {
            // id에 해당하는 패킷 핸들러 등록
            mIdToHandler[static_cast_16(id)] = [handler](const PacketView& packet) -> Bool
            {
                return HandlePayload<TPayload>(handler, packet);
            };
        }

    private:
        Bool                DispatchPacket(const PacketView& packet) { return mIdToHandler[packet.GetId()](packet); }

        template<typename TPayload, typename TPayloadHandler>
        static Bool         HandlePayload(TPayloadHandler handler, const PacketView& packet)
        {
            TPayload payload;
            if (!payload.ParseFromArray(packet.GetPayload(), packet.GetSize() - sizeof_16(PacketHeader)))
            {
                return false;
            }

            // 페이로드 처리
            return handler(packet.GetOwner(), payload);
        }

        static Bool         Handle_Invalid(const PacketView& packet);

    private:
        using PacketHandler     = Function<Bool(const PacketView&)>;

        PacketHandler       mIdToHandler[std::numeric_limits<Int16>::max() + 1];
    };
} // namespace protocol
