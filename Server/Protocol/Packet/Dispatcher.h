/*    Protocol/Packet/Dispatcher.h    */

#pragma once

#include "Protocol/Packet/Type.h"

namespace core
{
    class Session;
} // namespace core

namespace proto
{
    // 패킷 ID를 기반으로 들어오는 패킷을 지정된 핸들러 함수에 전달
    class PacketDispatcher
    {
    public:
        Int64               DispatchPackets(const SharedPtr<core::Session>& owner, const Byte* buffer, Int64 numBytes);

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

        PacketHandler           mIdToHandler[std::numeric_limits<Int16>::max() + 1];
    };
} // namespace protocol
