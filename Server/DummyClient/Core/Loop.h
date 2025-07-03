/*    DummyClient/Core/Loop.h    */

#pragma once

#include "Protocol/Packet/Queue.h"

namespace proto
{
    class PacketDispatcher;
}

namespace dummy
{
    class Loop
    {
    public:
        static constexpr MilliSec TickInterval = MilliSec(50); // 틱 간격
        static constexpr MilliSec MaxPacketProcessTime = TickInterval; // 최대 패킷 처리 시간

    public:
        static Loop& GetInstance()
        {
            static Loop sInstance; // 싱글턴 인스턴스
            return sInstance;
        }

        Loop(const Loop&) = delete; // 복사 생성자 삭제
        Loop& operator=(const Loop&) = delete; // 대입 연산자 삭제

        /**
         * 루프를 실행합니다.
         *
         * @param pktDispatcher 패킷 디스패처
         */
        void Run();

        /**
         * 루프를 중지합니다.
         */
        void Stop();

        /**
         * 버퍼의 패킷들을 큐에 추가합니다.
         *
         * @param owner 패킷 소유자 세션
         * @param buffer 패킷 데이터 버퍼
         * @param numBytes 버퍼에 있는 데이터 크기 (바이트 단위)
         * @return 버퍼의 패킷 중 큐로 추가된 패킷 크기의 합 (바이트 단위)
         */
        Int64 PushPackets(const SharedPtr<core::Session>& owner, const Byte* buffer, Int64 numBytes);

    private:
        Loop() = default; // 외부 생성 방지

        /**
         * 패킷을 처리합니다.
         *
         * @param pktDispatcher 패킷 디스패처
         */
        void ProcessPackets();

    private:
        proto::PacketQueue mPacketQueue; // 패킷 큐
        Bool mRunning = true; // 루프 실행 여부
    };
}
