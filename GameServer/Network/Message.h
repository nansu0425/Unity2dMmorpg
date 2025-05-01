/*    GameServer/Network/Message.h    */

#pragma once

#include "ServerEngine/Network/Message.h"
#include "Common/MessageData/Generated/Server_generated.h"

enum ServerMessageId : MessageId
{
    ServerMessageId_Invalid = 0x0000,
    ServerMessageId_Test,
};

class ClientMessageHandlerManager
    : public MessageHandlerManager
{

};

extern ClientMessageHandlerManager      gMessageHandlerManager;
