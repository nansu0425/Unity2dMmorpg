/*    DummyClient/Network/PacketHandler.h    */

#pragma once

#include "DummyClient/Network/Protocol/Packet.h"

class S2C_PacketHandlerMap
    : public PacketHandlerMap
{
public:
    static S2C_PacketHandlerMap&    GetInstance()
    {
        static S2C_PacketHandlerMap sInstance;
        return sInstance;
    }

protected:
    S2C_PacketHandlerMap() { RegisterAllHandlers(); }

    virtual void    RegisterAllHandlers() override
    {
        RegisterHandler(
            [this](SharedPtr<Session> session, const Byte* buffer, Int64 numBytes)
            {
                return HandlePayload<S2C_EnterRoom>(S2C_PacketHandlerMap::Handle_S2C_EnterRoom, std::move(session), buffer, numBytes);
            },
            PacketId::S2C_EnterRoom);

        RegisterHandler(
            [this](SharedPtr<Session> session, const Byte* buffer, Int64 numBytes)
            {
                return HandlePayload<S2C_Chat>(S2C_PacketHandlerMap::Handle_S2C_Chat, std::move(session), buffer, numBytes);
            },
            PacketId::S2C_Chat);
    }

private:    // 모든 페이로드 핸들러
    static Bool     Handle_S2C_EnterRoom(SharedPtr<Session> session, const S2C_EnterRoom& payload);
    static Bool     Handle_S2C_Chat(SharedPtr<Session> session, const S2C_Chat& payload);
};
