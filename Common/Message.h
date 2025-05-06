/*    Common/Message.h    */

#pragma once

#include "MessageData/Server_generated.h"
#include "MessageData/Client_generated.h"

// 서버에서 보내는 메시지 ID
enum class ServerMessageId : MessageId
{
    Login = 10'000,
    EnterGame,
    Chat,
};

// 클라이언트에서 보내는 메시지 ID
enum class ClientMessageId : MessageId
{
    Login = 20'000,
    EnterGame,
    Chat,
};
