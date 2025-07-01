/*    GameLogic/Command/Interface.h    */

#pragma once

namespace game
{
    class World;

    class ICommand
    {
    public:
        virtual ~ICommand() = default;

        /**
         * 명령을 실행합니다.
         *
         * @param world 명령이 실행될 월드 객체
         */
        virtual void Execute(World& world) = 0;
    };
}
