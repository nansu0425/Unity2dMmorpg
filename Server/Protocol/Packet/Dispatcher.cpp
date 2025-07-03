/*    Protocol/Packet/Dispatcher.cpp    */

#include "Protocol/Pch.h"
#include "Protocol/Packet/Dispatcher.h"
#include "Core/Network/Session.h"

using namespace core;

namespace proto
{
    PacketDispatcher::PacketDispatcher()
    {
        // Invalid 핸들러로 초기화
        for (Int32 i = 0; i < std::numeric_limits<Int16>::max() + 1; ++i)
        {
            mIdToHandler[i] = &PacketDispatcher::Handle_Invalid;
        }
    }

    Bool PacketDispatcher::Handle_Invalid(const SharedPtr<RawPacket>& packet)
    {
        gLogger->Error(TEXT_8("Session[{}]: Invalid packet id: {}"), packet->GetOwner()->GetId(), packet->GetId());
        return false;
    }
} // namespace protocol
