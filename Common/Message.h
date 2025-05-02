/*    Common/Message.h    */

#pragma once

#include "MessageData/Server_generated.h"
#include "MessageData/Client_generated.h"

// 서버에서 보내는 메시지 ID
enum ServerMessageId : MessageId
{
    ServerMessageId_Test = 10'000,
    ServerMessageId_Login = 10'001,
};

// 클라이언트에서 보내는 메시지 ID
enum ClientMessageId : MessageId
{
    ClientMessageId_Login = 20'000,
};
