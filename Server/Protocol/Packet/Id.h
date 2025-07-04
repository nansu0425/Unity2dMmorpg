/*    Protocol/Packet/Id.h    */

#pragma once

#include "Protocol/Payload/C2S.pb.h"
#include "Protocol/Payload/S2C.pb.h"

namespace proto
{
    enum class PacketId : Int16
    {
        Invalid = 0,
        C2S_EnterRoom = 1000,
        C2S_Chat = 1001,
        S2C_EnterRoom = 1002,
        S2C_Chat = 1003,
    };
} // namespace proto