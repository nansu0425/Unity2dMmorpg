/*    DummyClient/Network/Message.h    */

#pragma once

#include "ServerEngine/Network/Message.h"
#include "Common/MessageData/Generated/Server_generated.h"

enum class ServerMessageId : MessageId
{
    Invalid = 0x0000,
    Test,
};

class ServerMessageHandlerManager
    : public MessageHandlerManager
{
public:
    ServerMessageHandlerManager();

private:
    Bool    HandleTest(SharedPtr<MessageSession> session, const MessageData::Server::Test* data);
};

extern ServerMessageHandlerManager      gMessageHandlerManager;
