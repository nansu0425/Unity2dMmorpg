/*    ServerEngine/Network/Message.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Network/Message.h"

MessageHandlerManager::MessageHandlerManager()
{
    // 모든 핸들러를 HandleInvalid로 초기화
    for (MessageId i = 0; i < std::numeric_limits<MessageId>::max(); ++i)
    {
        mHandlers[i] = [this](SharedPtr<MessageSession> session, Byte* message, Int64 size)
            {
                return HandleInvalid(std::move(session), message, size);
            };
    }
}

Bool MessageHandlerManager::HandleMessage(SharedPtr<MessageSession> session, Byte* message, Int64 size)
{
    MessageHeader* header = reinterpret_cast<MessageHeader*>(message);
    // 메시지 id에 해당하는 핸들러 호출
    return mHandlers[header->id](std::move(session), message, size);
}

Bool MessageHandlerManager::HandleInvalid(SharedPtr<MessageSession> session, Byte* message, Int64 size)
{
    gLogger->Error(TEXT_16("Invalid message: id={}, size={}"), reinterpret_cast<MessageHeader*>(message)->id, size);
    return true;
}

uint8_t* MessageBuilder::Allocator::allocate(size_t dataSize)
{
    message = gSendBufferManager->Open(SIZE_64(MessageHeader) + dataSize);
    return message->GetBuffer() + SIZE_64(MessageHeader);
}

MessageBuilder::MessageBuilder()
    : mDataBuilder(kDataBufferSize)
{}

SharedPtr<SendBuffer> MessageBuilder::Finish(MessageId id)
{
    const Int16 dataSize = static_cast<Int16>(mDataBuilder.GetSize());
    const Int16 messageSize = SIZE_16(MessageHeader) + dataSize;
    SharedPtr<SendBuffer> message = gSendBufferManager->Open(messageSize);
    // 헤더 설정
    MessageHeader* header = reinterpret_cast<MessageHeader*>(message->GetBuffer());
    header->size = messageSize;
    header->id = id;
    // 메시지 데이터 복사
    ::memcpy(header + 1, mDataBuilder.GetBufferPointer(), dataSize);
    message->Close(messageSize);

    return message;
}
