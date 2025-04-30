/*    GameServer/Network/Message.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/Message.h"
#include "ServerEngine/Network/Session.h"

MessageHandlerManager    gMessageHandlerManager;

MessageHandlerManager::MessageHandlerManager()
{
    Init();
}

void MessageHandlerManager::Init()
{
    for (Int16 i = 0; i < std::numeric_limits<Int16>::max(); ++i)
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

SharedPtr<SendBuffer> MessageBuilder::Build(const flatbuffers::FlatBufferBuilder& fbb, Int16 messageId)
{
    const Int16 dataSize = static_cast<Int16>(fbb.GetSize());
    const Int16 messageSize = SIZE_16(MessageHeader) + dataSize;

    SharedPtr<SendBuffer> buffer = gSendBufferManager->Open(messageSize);
    // 헤더 설정
    MessageHeader* header = reinterpret_cast<MessageHeader*>(buffer->GetBuffer());
    header->size = messageSize;
    header->id = messageId;
    // 데이터 복사
    ::memcpy(header + 1, fbb.GetBufferPointer(), dataSize);
    buffer->Close(messageSize);

    return buffer;
}
