/*    Protocol/Packet/Id.h    */

#pragma once

namespace proto
{
    enum class PacketId : Int16
    {
        Invalid = 0,
        WorldToClient_EnterRoom = 1000,
        WorldToClient_Chat = 1001,
        ClientToWorld_EnterRoom = 1002,
        ClientToWorld_Chat = 1003,
    };
} // namespace proto