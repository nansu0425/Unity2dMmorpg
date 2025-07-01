/*    GameLogic/Command/Queue.h    */

#pragma once

namespace game
{
    class World;
    class ICommand;

    class CommandQueue
    {
    public:
        CommandQueue() = default;
        ~CommandQueue() = default;

        /**
         * 명령을 큐에 추가합니다.
         *
         * @param command 추가할 명령
         */
        void Push(const SharedPtr<ICommand>& command);

        /**
         * 큐에서 명령을 가져와 실행합니다.
         *
         * @param world 명령이 실행될 월드 객체
         * @return 실행된 명령의 수
         */
        Int64 Execute(World& world);

    private:
        LockfreeQueue<SharedPtr<ICommand>> mQueue; // 명령 큐
    };
}
