/*    ServerEngine/Network/Message.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Network/Message.h"

MessageHandlerManager::MessageHandlerManager()
{
    // 모든 핸들러를 HandleInvalid로 초기화
    for (MessageId i = 0; i < std::numeric_limits<MessageId>::max(); ++i)
    {
        mHandlers[i] = [this](SharedPtr<Session> session, MessageHeader* header)
            {
                return HandleInvalid(std::move(session), header);
            };
    }
}

Bool MessageHandlerManager::HandleMessage(SharedPtr<Session> session, MessageHeader* header)
{
    // 메시지 id에 해당하는 핸들러 호출
    return mHandlers[header->id](std::move(session), header);
}

Bool MessageHandlerManager::HandleInvalid(SharedPtr<Session> session, MessageHeader* header)
{
    gLogger->Error(TEXT_16("Invalid message: id={}, size={}"), header->id, header->size);
    return true;
}

NetMessage::NetMessage(MessageId id)
    : mDataBuilder(kDataBufferSize)
{
    mHeader.id = id;
}
