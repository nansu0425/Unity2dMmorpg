/*    WorldServer/Network/Session.h    */

#pragma once

#include "Core/Network/Session.h"

namespace game
{
    class Room;
} // namespace game

namespace world
{
    class ClientSession
        : public core::Session
    {
    public:
        virtual ~ClientSession() override;

        void                SetPlayerId(Int64 id) { mPlayerId = id; }
        Int64               GetPlayerId() const { return mPlayerId; }

    protected:
        virtual void        OnConnected() override;
        virtual void        OnDisconnected(String8 cause) override;
        virtual Int64       OnReceived(const Byte* buffer, Int64 numBytes) override;
        virtual void        OnSent(Int64 numBytes) override;

    private:
        Int64               mPlayerId = 0;
    };

    extern SharedPtr<game::Room>    gRoom;
} // namespace world
