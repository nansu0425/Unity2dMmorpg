/*    GameServer/Network/Message.h    */

#pragma once

#include "ServerEngine/Network/Message.h"
#include "Common/Message.h"

class ClientMessageHandlerManager
    : public MessageHandlerManager
{
public:
    virtual void    RegisterAllHandlers() override;

private:
    Bool            HandleLogin(SharedPtr<Session> session, const MessageData::Client::Login* data);
    Bool            HandleEnterGame(SharedPtr<Session> session, const MessageData::Client::EnterGame* data);
    Bool            HandleChat(SharedPtr<Session> session, const MessageData::Client::Chat* data);
};

extern ClientMessageHandlerManager      gMessageHandlerManager;
