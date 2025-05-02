/*    ServerEngine/Network/Message.h    */

#pragma once

#include <flatbuffers/flatbuffers.h>

class Session;

using MessageId = Int16;

struct MessageHeader
{
    Int16       size;   // 헤더까지 포함한 메시지의 전체 크기
    MessageId   id;     // 메시지 식별자
};

class ReceiveMessage
{
public:
    explicit ReceiveMessage(MessageHeader* header);

    MessageHeader*      GetHeader() { return mHeader; }
    Int16               GetSize() const { return mHeader->size; }
    MessageId           GetId() const { return mHeader->id; }
    Byte*               GetData() { return mData; }

private:
    MessageHeader*      mHeader = nullptr;
    Byte*               mData = nullptr;
};

class MessageHandlerManager
{
public:
    using MessageHandler    = Function<Bool(SharedPtr<Session>, ReceiveMessage)>;

public:
    MessageHandlerManager();

    Bool                HandleMessage(SharedPtr<Session> session, ReceiveMessage message);
    // 여기서 모든 메시지 핸들러를 등록해야 한다
    virtual void        RegisterAllHandlers() = 0;

protected:
    MessageHandler&     GetHandler(MessageId id) { return mHandlers[id]; }
    void                RegisterHandler(MessageId id, MessageHandler handler) { mHandlers[id] = std::move(handler); }

private:
    Bool                HandleInvalid(SharedPtr<Session> session, ReceiveMessage message);

private:
    MessageHandler      mHandlers[std::numeric_limits<MessageId>::max()] = {};
};

class SendMessageBuilder
{
public:
    enum Constants : Int16
    {
        kDataBufferSize = 0x1000,   // 메시지의 헤더를 제외한 데이터를 담을 수 있는 버퍼 크기
    };

public:
    SendMessageBuilder(MessageId id);

    template<typename T>
    void FinishBuilding(flatbuffers::Offset<T> rootData)
    {
        ASSERT_CRASH(!mIsBuilt, "MESSAGE_ALREADY_BUILT");
        mDataBuilder.Finish(rootData);
        ASSERT_CRASH(GetDataSize() <= kDataBufferSize, "MESSAGE_SIZE_OVERFLOW");

        mHeader.size = SIZE_16(MessageHeader) + static_cast<Int16>(mDataBuilder.GetSize());
        mIsBuilt = true;
    }

public:
    MessageHeader&                      GetHeader() { return mHeader; }
    flatbuffers::FlatBufferBuilder&     GetDataBuilder() { return mDataBuilder; }
    Byte*                               GetDataBuffer() { return mDataBuilder.GetBufferPointer(); }
    Int64                               GetDataSize() const { return mDataBuilder.GetSize(); }
    Bool                                IsBuilt() const { return mIsBuilt; }

private:
    MessageHeader                       mHeader = {};
    flatbuffers::FlatBufferBuilder      mDataBuilder;
    Bool                                mIsBuilt = false;
};
