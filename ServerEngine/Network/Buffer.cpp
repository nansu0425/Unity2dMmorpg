/*    ServerEngine/Network/Buffer.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Network/Buffer.h"
#include "ServerEngine/Network/Message.h"

ReceiveBuffer::ReceiveBuffer(Int64 size)
    : mSize(size)
    , mCapacity(size * kCapcityFactor)
{
    mBuffer.resize(mCapacity);
}

ReceiveBuffer::~ReceiveBuffer()
{}

void ReceiveBuffer::Clear()
{
    Int64 dataSize = GetDataSize();
    // 모든 데이터를 읽은 상태
    if (dataSize == 0)
    {
        mReadPos = 0;
        mWritePos = 0;
    }
    // 여유 공간 크기가 버퍼 크기보다 작아졌는데도 읽지 않은 데이터가 존재
    else if (GetFreeSize() < mSize)
    {
        // 데이터 앞으로 이동
        ::memcpy(mBuffer.data(), mBuffer.data() + mReadPos, dataSize);
        mReadPos = 0;
        mWritePos = dataSize;
    }
}

Bool ReceiveBuffer::OnRead(Int64 numBytes)
{
    // 유효한 데이터보다 더 많이 읽은 경우
    if (numBytes > GetDataSize())
    {
        return false;
    }
    // 읽은 만큼 읽기 위치를 이동
    mReadPos += numBytes;

    return true;
}

Bool ReceiveBuffer::OnWritten(Int64 numBytes)
{
    // 여유 공간보다 더 많이 쓴 경우
    if (numBytes > GetFreeSize())
    {
        return false;
    }
    // 쓴 만큼 쓰기 위치를 이동
    mWritePos += numBytes;

    return true;
}

void SendBuffers::PushMessage(SharedPtr<SendMessageBuilder> message)
{
    ASSERT_CRASH(message->IsBuilt(), "MESSAGE_NOT_BUILT");
    mMessages.push_back(std::move(message));
    // 메시지 헤더
    WSABUF header;
    header.buf = reinterpret_cast<CHAR*>(&mMessages.back()->GetHeader());
    header.len = SIZE_16(MessageHeader);
    mBuffers.push_back(header);
    // 메시지 데이터
    WSABUF data;
    data.buf = reinterpret_cast<CHAR*>(mMessages.back()->GetDataBuffer());
    data.len = static_cast<ULONG>(mMessages.back()->GetDataSize());
    mBuffers.push_back(data);
}

void SendBuffers::Clear()
{
    mBuffers.clear();
    mMessages.clear();
}

//SendBuffer::SendBuffer(SharedPtr<SendBufferChunk> owner, Byte* buffer, Int64 allocSize)
//    : mBuffer(buffer)
//    , mAllocSize(allocSize)
//    , mOwner(std::move(owner))
//{}
//
//SendBuffer::~SendBuffer()
//{}
//
//void SendBuffer::Close(Int64 writtenSize)
//{
//    ASSERT_CRASH(writtenSize <= mAllocSize, "BUFFER_OVERFLOW");
//    mWrittenSize = writtenSize;
//    mOwner->Close(mWrittenSize);
//}
//
//SharedPtr<SendBuffer> SendBufferManager::Open(Int64 allocSize)
//{
//    if (tSendBufferChunk == nullptr)
//    {
//        tSendBufferChunk = Pop();
//        tSendBufferChunk->Clear();
//    }
//
//    ASSERT_CRASH(tSendBufferChunk->IsOpen() == false, "ALREADY_OPEND");
//
//    // 더 이상 쓰기를 할 수 있는 여유 공간이 없으면 새로운 청크로 교체
//    if (tSendBufferChunk->GetFreeSize() < allocSize)
//    {
//        tSendBufferChunk = Pop();
//        tSendBufferChunk->Clear();
//    }
//
//    return tSendBufferChunk->Open(allocSize);
//}
//
//SharedPtr<SendBufferChunk> SendBufferManager::Pop()
//{
//    {
//        WRITE_GUARD;
//        // 사용 가능한 청크가 있는 경우
//        if (false == mSendChunks.empty())
//        {
//            SharedPtr<SendBufferChunk> chunk = mSendChunks.back();
//            mSendChunks.pop_back();
//            return chunk;
//        }
//    }
//    // 새로운 청크 할당
//    return SharedPtr<SendBufferChunk>(new SendBufferChunk(), PushGlobal);
//}
//
//void SendBufferManager::Push(SharedPtr<SendBufferChunk> chunk)
//{
//    WRITE_GUARD;
//    mSendChunks.push_back(chunk);
//}
//
//void SendBufferManager::PushGlobal(SendBufferChunk* chunk)
//{
//    // 청크를 재사용
//    // gSendBufferManager->Push(SharedPtr<SendBufferChunk>(chunk, PushGlobal));
//}
//
//SendBufferChunk::SendBufferChunk()
//{}
//
//SendBufferChunk::~SendBufferChunk()
//{}
//
//SharedPtr<SendBuffer> SendBufferChunk::Open(Int64 allocSize)
//{
//    ASSERT_CRASH(allocSize <= GetFreeSize(), "BUFFER_OVERFLOW");
//    ASSERT_CRASH(mIsOpen == false, "ALREADY_OPEND");
//
//    mIsOpen = true;
//
//    return std::make_shared<SendBuffer>(shared_from_this(), AtWritePos(), allocSize);
//}
//
//void SendBufferChunk::Close(Int64 writtenSize)
//{
//    ASSERT_CRASH(mIsOpen == true, "NOT_OPEND");
//    mIsOpen = false;
//    mWritePos += writtenSize;
//}
//
//void SendBufferChunk::Clear()
//{
//    mIsOpen = false;
//    mWritePos = 0;
//}

BufferReader::BufferReader()
{}

BufferReader::BufferReader(Byte* buffer, Int64 size, Int64 pos)
    : mBuffer(buffer)
    , mSize(size)
    , mPos(pos)
{}

BufferReader::~BufferReader()
{}

Bool BufferReader::Peek(Byte* dest, Int64 size)
{
    if (GetFreeSize() < size)
    {
        return false;
    }
    ::memcpy(dest, mBuffer + mPos, size);

    return true;
}

Bool BufferReader::Read(Byte* dest, Int64 size)
{
    if (Peek(dest, size) == false)
    {
        return false;
    }
    mPos += size;

    return true;
}

BufferWriter::BufferWriter()
{}

BufferWriter::BufferWriter(Byte* buffer, Int64 size, Int64 pos)
    : mBuffer(buffer)
    , mSize(size)
    , mPos(pos)
{}

BufferWriter::~BufferWriter()
{}

Bool BufferWriter::Write(const Byte* src, Int64 size)
{
    if (GetFreeSize() < size)
    {
        return false;
    }
    ::memcpy(mBuffer + mPos, src, size);
    mPos += size;

    return true;
}
