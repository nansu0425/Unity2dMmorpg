/*    GameLogic/Core/Loop.h    */

#pragma once

#include "GameLogic/Command/Queue.h"
#include "GameLogic/Core/World.h"

namespace game
{
    class Loop
    {
    public:
        static constexpr MilliSec TickInterval = MilliSec(50); // 틱 간격 (50ms)

    public:
        /**
         * 루프를 실행합니다.
         */
        void Run();

        /**
         * 루프를 중지합니다.
         */
        void Stop();

        /**
         * 명령 큐에 명령을 추가합니다.
         *
         * @param command 추가할 명령
         */
        void PushCommand(const SharedPtr<ICommand>& command);

    private:
        /**
         * 월드 객체를 업데이트합니다.
         */
        void UpdateWorld();

        /**
         * 명령 큐에서 명령을 실행합니다.
         */
        void ExecuteCommands();

        /**
         * 타이머를 처리합니다.
         */
        void HandleTimers();

    private:
        World mWorld; // 월드 객체
        CommandQueue mCommandQueue; // 명령 큐
        Bool mRunning = true; // 루프 실행 여부
    };
}
