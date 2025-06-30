/*    Protocol/Packet/Queue.h    */

#pragma once

namespace core
{
    class Session;
}

namespace proto
{
    struct RawPacket;

    class PacketQueue
    {
    public:
        PacketQueue();
        ~PacketQueue();

        /**
         * 패킷을 큐에 추가합니다.
         *
         * @param packet 추가할 패킷
         */
        void Push(const SharedPtr<RawPacket>& packet);

        /**
         * 큐에서 패킷을 가져옵니다.
         *
         * @return 가져온 패킷, 큐가 비어있으면 nullptr
         */
        SharedPtr<RawPacket> PopBlocking();

    private:
        LockfreeQueue<SharedPtr<RawPacket>> mQueue; // 패킷 큐
        HANDLE mSemaphore; // 큐 동기화를 위한 세마포어
    };
}
