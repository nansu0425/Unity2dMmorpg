/*    GameLogic/Command/Queue.cpp    */

#include "GameLogic/Pch.h"
#include "GameLogic/Command/Queue.h"
#include "GameLogic/Core/World.h"
#include "GameLogic/Interface.h"

namespace game
{
    void CommandQueue::Push(const SharedPtr<ICommand>& command)
    {
        while (!mQueue.enqueue(command))
        {
            // 큐가 가득 찬 경우 대기합니다.
            _mm_pause();
        }
    }

    Int64 CommandQueue::Execute(World& world)
    {
        Int64 executedCount = 0;
        SharedPtr<ICommand> command;

        while (mQueue.try_dequeue(command))
        {
            if (command)
            {
                command->Execute(world);
                ++executedCount;
            }
        }

        return executedCount;
    }
}
