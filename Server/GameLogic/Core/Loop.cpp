/*    GameLogic/Core/Loop.cpp    */

#include "GameLogic/Pch.h"
#include "GameLogic/Core/Loop.h"

namespace game
{
    void Loop::Run()
    {
        Int64 tickCount = 0;
        auto lastLogTime = std::chrono::steady_clock::now();

        while (mRunning)
        {
            auto start = std::chrono::steady_clock::now();

            ExecuteCommands();
            UpdateWorld();
            HandleTimers();

            ++tickCount;

            // 1초마다 틱 카운트 로그 출력
            auto now = std::chrono::steady_clock::now();
            auto elapsedSinceLastLog = std::chrono::duration_cast<Seconds>(now - lastLogTime);
            if (elapsedSinceLastLog >= Seconds(1))
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

    void Loop::PushCommand(const SharedPtr<ICommand>& command)
    {
        if (command)
        {
            mCommandQueue.Push(command);
        }
    }

    void Loop::UpdateWorld()
    {}

    void Loop::ExecuteCommands()
    {
        mCommandQueue.Execute(mWorld);
    }

    void Loop::HandleTimers()
    {}
}
