/*    GameServer/Network/Message.h    */

#pragma once

#include "ServerEngine/Network/Message.h"
#include "Common/MessageData/Generated/Server_generated.h"

enum class ServerMessageId : MessageId
{
    Invalid = 0x0000,
    Test,
};

class ClientMessageHandlerManager
    : public MessageHandlerManager
{

};

extern ClientMessageHandlerManager      gMessageHandlerManager;
