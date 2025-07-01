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
        static Loop& GetInstance()
        {
            static Loop sInstance; // 싱글턴 인스턴스
            return sInstance;
        }

        Loop(const Loop&) = delete; // 복사 생성자 삭제
        Loop& operator=(const Loop&) = delete; // 대입 연산자 삭제

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
        Loop() = default; // 외부 생성 방지

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
