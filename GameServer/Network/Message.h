/*    GameServer/Network/Message.h    */

#pragma once

#include "ServerEngine/Network/Message.h"
#include "Common/Message.h"

class ClientMessageHandlerManager
    : public MessageHandlerManager
{
public:
    ClientMessageHandlerManager();

private:
    Bool    HandleLogin(SharedPtr<Session> session, const MessageData::Client::Login* data);
};

extern ClientMessageHandlerManager      gMessageHandlerManager;
