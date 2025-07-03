/*    Protocol/Packet/Queue.h    */

#pragma once

namespace core
{
    class Session;
}

namespace proto
{
    class RawPacket;

    class PacketQueue
    {
    public:
        /**
         * 패킷을 큐에 추가합니다.
         *
         * @param packet 추가할 패킷
         */
        void Push(const SharedPtr<RawPacket>& packet);

        /**
         * 버퍼의 패킷들을 큐에 추가합니다.
         *
         * @param owner 패킷 소유자 세션
         * @param buffer 패킷 데이터 버퍼
         * @param numBytes 버퍼에 있는 데이터 크기 (바이트 단위)
         * @return 버퍼의 패킷 중 큐로 추가된 패킷 크기의 합 (바이트 단위)
         */
        Int64 Push(const SharedPtr<core::Session>& owner, const Byte* buffer, Int64 numBytes);

        /**
         * 큐에서 패킷을 가져옵니다.
         *
         * @param packet 가져온 패킷을 저장할 변수
         * @return 성공 여부
         */
        Bool TryPop(SharedPtr<RawPacket>& packet);

    private:
        LockfreeQueue<SharedPtr<RawPacket>> mQueue; // 패킷 큐
    };
}
