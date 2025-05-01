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
    enum Constants : Int16
    {
        kDataBufferSize = 0x1000,   // 메시지의 헤더를 제외한 데이터를 담을 수 있는 버퍼 크기
    };

    struct Allocator
        : public flatbuffers::Allocator
    {
        SharedPtr<SendBuffer>   message;    // 메시지를 담을 버퍼

        virtual uint8_t*        allocate(size_t dataSize) override;
        virtual void            deallocate(uint8_t* data, size_t size) override { gLogger->Warn(TEXT_8("Deallocated")); }
    };

public:
    MessageBuilder();

    SharedPtr<SendBuffer>               Finish(MessageId id);

public:
    flatbuffers::FlatBufferBuilder&     GetDataBuilder() { return mDataBuilder; }

private:
    flatbuffers::FlatBufferBuilder      mDataBuilder;
};
