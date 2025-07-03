/*    Protocol/Packet/Dispatcher.h    */

#pragma once

#include "Protocol/Packet/Type.h"
#include "Protocol/Packet/Queue.h"

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
        /**
         * 패킷을 핸들러로 전달합니다.
         *
         * @param packet 핸들러로 전달할 패킷
         * @return 패킷이 성공적으로 처리되었는지 여부
         */
        Bool                DispatchPacket(const SharedPtr<RawPacket>& packet) { return mIdToHandler[packet->GetId()](packet); }

    protected:
                            PacketDispatcher();
        // PacketDispatcher를 상속받은 클래스에서 RegisterHandler()로 모든 패킷 핸들러 등록
        virtual void        RegisterAllHandlers() = 0;

        template<typename TPayload, typename TPayloadHandler>
        void                RegisterHandler(TPayloadHandler handler, PacketId id)
        {
            // id에 해당하는 패킷 핸들러 등록
            mIdToHandler[static_cast_16(id)] = [handler](const SharedPtr<RawPacket>& packet) -> Bool
            {
                return HandlePayload<TPayload>(handler, packet);
            };
        }

    private:
        template<typename TPayload, typename TPayloadHandler>
        static Bool         HandlePayload(TPayloadHandler handler, const SharedPtr<RawPacket>& packet)
        {
            TPayload payload;
            if (!payload.ParseFromArray(packet->GetPayload(), packet->GetSize() - sizeof_16(PacketHeader)))
            {
                return false;
            }

            // 페이로드 처리
            return handler(packet->GetOwner(), payload);
        }

        static Bool         Handle_Invalid(const SharedPtr<RawPacket>& packet);

    private:
        using PacketHandler     = Function<Bool(const SharedPtr<RawPacket>&)>;

        PacketHandler           mIdToHandler[std::numeric_limits<Int16>::max() + 1];
    };
} // namespace protocol
