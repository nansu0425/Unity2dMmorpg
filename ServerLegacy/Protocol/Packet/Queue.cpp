/*    Protocol/Packet/Queue.cpp    */

#include "Protocol/Pch.h"
#include "Protocol/Packet/Queue.h"
#include "Protocol/Packet/Type.h"

namespace proto
{
    PacketQueue::PacketQueue()
    {
        // 큐를 초기화하고 세마포어를 생성합니다.
        mSemaphore = CreateSemaphore(nullptr, 0, LONG_MAX, nullptr);
    }

    PacketQueue::~PacketQueue()
    {
        // 세마포어를 닫습니다.
        if (mSemaphore != nullptr)
        {
            CloseHandle(mSemaphore);
            mSemaphore = nullptr;
        }
    }

    void PacketQueue::Push(const SharedPtr<RawPacket>& packet)
    {
        // 큐에 패킷을 추가합니다.
        while (!mQueue.enqueue(packet))
        {
            // 큐가 가득 찬 경우 대기합니다.
            _mm_pause();
        }
        // 패킷이 추가되었음을 알리기 위해 세마포어를 증가시킵니다.
        ReleaseSemaphore(mSemaphore, 1, nullptr);
    }

    SharedPtr<RawPacket> PacketQueue::PopBlocking()
    {
        // 패킷이 큐에 추가될 때까지 대기합니다.
        WaitForSingleObject(mSemaphore, INFINITE);
        // 큐에서 패킷을 가져옵니다.
        SharedPtr<RawPacket> packet;
        while (!mQueue.try_dequeue(packet))
        {
            // 큐가 비어있으면 대기합니다.
            _mm_pause();
        }
        // 가져온 패킷을 반환합니다.
        return packet;
    }
}
