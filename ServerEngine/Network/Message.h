/*    ServerEngine/Network/Message.h    */

#pragma once

#include <flatbuffers/flatbuffers.h>

class MessageSession;

using MessageId = Int16;

struct MessageHeader
{
    Int16       size;   // 헤더까지 포함한 메시지의 전체 크기
    MessageId   id;     // 메시지 식별자
};

class MessageHandlerManager
{
public:
    using MessageHandler    = Function<Bool(SharedPtr<MessageSession>, Byte*, Int64)>;

public:
    MessageHandlerManager();

    Bool                HandleMessage(SharedPtr<MessageSession> session, Byte* message, Int64 size);

protected:
    MessageHandler&     GetHandler(MessageId id) { return mHandlers[id]; }
    void                RegisterHandler(MessageId id, MessageHandler handler) { mHandlers[id] = std::move(handler); }

private:
    Bool                HandleInvalid(SharedPtr<MessageSession> session, Byte* message, Int64 size);

private:
    MessageHandler      mHandlers[std::numeric_limits<MessageId>::max()] = {};
};

class MessageBuilder
{
public:
    static SharedPtr<SendBuffer>    Build(const flatbuffers::FlatBufferBuilder& fbb, MessageId messageId);
};
