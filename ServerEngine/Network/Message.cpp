/*    ServerEngine/Network/Message.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Network/Message.h"

ReceiveMessage::ReceiveMessage(MessageHeader* header)
    : mHeader(header)
    , mData(reinterpret_cast<Byte*>(header + 1))
{}

MessageHandlerManager::MessageHandlerManager()
{
    // 모든 핸들러를 HandleInvalid로 초기화
    for (MessageId i = 0; i < std::numeric_limits<MessageId>::max(); ++i)
    {
        mHandlers[i] = [this](SharedPtr<Session> session, ReceiveMessage message)
            {
                return HandleInvalid(std::move(session), message);
            };
    }
}

Bool MessageHandlerManager::HandleMessage(SharedPtr<Session> session, ReceiveMessage message)
{
    // 메시지 id에 해당하는 핸들러 호출
    return mHandlers[message.GetId()](std::move(session), message);
}

Bool MessageHandlerManager::HandleInvalid(SharedPtr<Session> session, ReceiveMessage message)
{
    gLogger->Error(TEXT_16("Invalid message: id={}, size={}"), message.GetId(), message.GetSize());
    return true;
}

SendMessageBuilder::SendMessageBuilder(MessageId id)
    : mDataBuilder(kDataBufferSize)
{
    mHeader.id = id;
}
