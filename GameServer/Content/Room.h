/*    GameServer/Content/Room.h    */

#pragma once

#include "GameServer/Content/Job.h"

struct Player;

class Room
{
public:
    void PushJob(SharedPtr<IJob> job) { mJobs.Push(std::move(job)); }
    void FlushJobs();

private:
    friend class EnterJob;
    friend class LeaveJob;
    friend class BroadcastJob;

    void Enter(SharedPtr<Player> player);
    void Leave(SharedPtr<Player> player);
    void Broadcast(SharedPtr<SendMessageBuilder> message);

private:
    TreeMap<Int64, SharedPtr<Player>>   mPlayers;
    JobQueue                            mJobs;
};

extern Room gRoom;

class EnterJob
    : public IJob
{
public:
    EnterJob(Room& room, SharedPtr<Player> player)
        : mRoom(room)
        , mPlayer(player)
    {}

    virtual void Execute() override
    {
        mRoom.Enter(mPlayer);
    }

private:
    Room&               mRoom;
    SharedPtr<Player>   mPlayer;
};

class LeaveJob
    : public IJob
{
public:
    LeaveJob(Room& room, SharedPtr<Player> player)
        : mRoom(room)
        , mPlayer(player)
    {}

    virtual void Execute() override
    {
        mRoom.Leave(mPlayer);
    }

private:
    Room&               mRoom;
    SharedPtr<Player>   mPlayer;
};

class BroadcastJob
    : public IJob
{
public:
    BroadcastJob(Room& room, SharedPtr<SendMessageBuilder> sendMsg)
        : mRoom(room)
        , mSendMsg(sendMsg)
    {}

    virtual void Execute() override
    {
        mRoom.Broadcast(mSendMsg);
    }

private:
    Room&                           mRoom;
    SharedPtr<SendMessageBuilder>   mSendMsg;
};
