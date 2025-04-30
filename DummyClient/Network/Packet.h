/*    DummyClient/Network/Packet..h    */

#pragma once

#include "Common/MessageData/Generated/Server_generated.h"

class PacketSession;

enum class ServerMessageId : Int16
{
    Invalid = 0x0000,
    Test,
};

class MessageHandlerManager
{
public:
    using MessageHandler    = Function<Bool(SharedPtr<PacketSession>, Byte*, Int64)>;

public:
    MessageHandlerManager();

    void            Init();
    Bool            HandleMessage(SharedPtr<PacketSession> session, Byte* message, Int64 size);

private:
    Bool            HandleInvalid(SharedPtr<PacketSession> session, Byte* message, Int64 size);
    Bool            HandleTest(SharedPtr<PacketSession> session, const MessageData::Server::Test* data);

private:
    MessageHandler  mHandlers[std::numeric_limits<Int16>::max()] = {};
};

extern MessageHandlerManager    gMessageHandlerManager;
