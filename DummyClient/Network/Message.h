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
    Bool            HandleLogin(SharedPtr<Session> session, const MessageData::Server::Login* data);
    Bool            HandleEnterGame(SharedPtr<Session> session, const MessageData::Server::EnterGame* data);
    Bool            HandleChat(SharedPtr<Session> session, const MessageData::Server::Chat* data);
};

extern ServerMessageHandlerManager      gMessageHandlerManager;
