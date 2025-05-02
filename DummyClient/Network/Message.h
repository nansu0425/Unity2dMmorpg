/*    DummyClient/Network/Message.h    */

#pragma once

#include "ServerEngine/Network/Message.h"
#include "Common/Message.h"

class ServerMessageHandlerManager
    : public MessageHandlerManager
{
public:
    virtual void    RegisterAllHandlers() override;

private:
    Bool            HandleTest(SharedPtr<Session> session, const MessageData::Server::Test* data);
    Bool            HandleLogin(SharedPtr<Session> session, const MessageData::Server::Login* data);
};

extern ServerMessageHandlerManager      gMessageHandlerManager;
