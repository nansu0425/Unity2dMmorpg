/*    DummyClient/Network/Session.h    */

#pragma once

#include "Core/Network/Session.h"

namespace game
{
    class Room;
} // namespace game

namespace dummy
{
    class ServerSession
        : public core::Session
    {
    public:
        virtual             ~ServerSession() override;

        SharedPtr<ServerSession> GetServerSession() { return std::static_pointer_cast<ServerSession>(shared_from_this()); }

    protected:
        virtual void        OnConnected() override;
        virtual void        OnDisconnected(String8 cause) override;
        virtual Int64       OnReceived(const Byte* buffer, Int64 numBytes) override;
        virtual void        OnSent(Int64 numBytes) override;
    };

    extern SharedPtr<game::Room>    gRoom;
} // namespace dummy
