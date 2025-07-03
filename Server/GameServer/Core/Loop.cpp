/*    GameServer/Core/Loop.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Core/Loop.h"
#include "GameServer/Packet/Handler.h"
#include "Core/Network/Session.h"

namespace game
{
    void Loop::Run()
    {
        Int64 tickCount = 0;
        auto lastLogTime = std::chrono::steady_clock::now();

        while (mRunning)
        {
            auto start = std::chrono::steady_clock::now();

            ProcessPackets();
            UpdateWorld();
            HandleTimers();

            ++tickCount;

            // 1초마다 틱 카운트 로그 출력
            auto now = std::chrono::steady_clock::now();
            auto elapsedSinceLastLog = std::chrono::duration_cast<MilliSec>(now - lastLogTime);
            if (elapsedSinceLastLog >= MilliSec(1000))
            {
                core::gLogger->Info("Tick Count: {}", tickCount);
                tickCount = 0;
                lastLogTime = now;
            }
            
            // 틱 간격 유지
            MilliSec elapsed;
            do
            {
                auto now = std::chrono::steady_clock::now();
                elapsed = std::chrono::duration_cast<MilliSec>(now - start);
            }
            while (elapsed < TickInterval);
        }
    }

    void Loop::Stop()
    {
        mRunning = false;
    }

    Int64 Loop::PushPackets(const SharedPtr<core::Session>& owner, const Byte* buffer, Int64 numBytes)
    {
        return mPacketQueue.Push(owner, buffer, numBytes);
    }

    void Loop::ProcessPackets()
    {
        auto start = std::chrono::steady_clock::now();

        SharedPtr<proto::RawPacket> packet;
        while (mPacketQueue.TryPop(packet))
        {
            ASSERT_CRASH_DEBUG(packet != nullptr, "NULL_PACKET_RECEIVED");

            // 패킷을 핸들러로 전달하여 처리
            Bool result = game::ToWorld_PacketHandler::GetInstance().DispatchPacket(packet);
            if (!result)
            {
                core::gLogger->Error(TEXT_8("Session[{}]: Failed to process packet with id: {}"), packet->GetOwner()->GetId(), packet->GetId());
            }

            // 최대 패킷 처리 시간을 넘겼는지 확인
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<MilliSec>(now - start);
            if (elapsed >= MaxPacketProcessTime)
            {
                core::gLogger->Warn(TEXT_8("Packet processing took too long: {} ms"), elapsed.count());
                break; // 패킷 처리 시간 초과 시 루프 종료
            }
        }
    }

    void Loop::UpdateWorld()
    {}

    void Loop::HandleTimers()
    {}
}
