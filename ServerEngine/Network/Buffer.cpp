/*    ServerEngine/Network/Buffer.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Network/Buffer.h"

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
    ASSERT_CRASH(0 <= numBytes, "INVALID_NUM_BYTES");

    // 유효한 데이터보다 더 많이 읽은 경우
    if (numBytes > GetDataSize())
    {
        gLogger->Error(TEXT_8("Failed to read receive buffer.: {} bytes"), numBytes);
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
        gLogger->Error(TEXT_8("Failed to write to receive buffer: {} bytes"), numBytes);
        return false;
    }
    // 쓴 만큼 쓰기 위치를 이동
    mWritePos += numBytes;

    return true;
}

SendBuffer::SendBuffer(SharedPtr<SendChunk> owner, Byte* buffer, Int64 allocSize)
    : mBuffer(buffer)
    , mAllocSize(allocSize)
    , mOwner(std::move(owner))
{}

SendBuffer::~SendBuffer()
{}

void SendBuffer::OnWritten(Int64 writtenSize)
{
    ASSERT_CRASH(writtenSize <= mAllocSize, "BUFFER_OVERFLOW");
    mWrittenSize = writtenSize;
    mOwner->OnWritten(mWrittenSize);
}

SharedPtr<SendBuffer> SendChunk::Alloc(Int64 allocSize)
{
    ASSERT_CRASH(allocSize <= GetFreeSize(), "BUFFER_OVERFLOW");
    ASSERT_CRASH(mIsWriting == false, "WRITING_STATE");

    mIsWriting = true;

    return std::make_shared<SendBuffer>(shared_from_this(), AtWritePos(), allocSize);
}

void SendChunk::OnWritten(Int64 writtenSize)
{
    ASSERT_CRASH(mIsWriting == true, "NOT_WRITING_STATE");
    mIsWriting = false;
    mWrittenSize += writtenSize;
}

void SendChunk::Clear()
{
    mIsWriting = false;
    mWrittenSize = 0;
}

SharedPtr<SendBuffer> SendChunkPool::Alloc(Int64 allocSize)
{
    // 현재 스레드에 청크가 없으면 가져온다
    if (tSendChunk == nullptr)
    {
        tSendChunk = Pop();
        tSendChunk->Clear();
    }

    ASSERT_CRASH(tSendChunk->IsWriting() == false, "WRITING_STATE");

    // 더 이상 쓰기를 할 수 있는 여유 공간이 없으면 새로운 청크로 교체
    if (tSendChunk->GetFreeSize() < allocSize)
    {
        tSendChunk = Pop();
        tSendChunk->Clear();
    }

    return tSendChunk->Alloc(allocSize);
}

void SendChunkPool::Delete(SendChunk* chunk)
{
    // 청크를 재사용하기 위해 풀링
    gSendChunkPool->Push(SharedPtr<SendChunk>(chunk, Delete));
}

SharedPtr<SendChunk> SendChunkPool::Pop()
{
    {
        WRITE_GUARD;
        // 사용 가능한 청크가 있는 경우
        if (mSendChunks.empty() == false)
        {
            SharedPtr<SendChunk> chunk = mSendChunks.back();
            mSendChunks.pop_back();
            return chunk;
        }
    }
    // 새로운 청크 할당
    return SharedPtr<SendChunk>(new SendChunk(), Delete);
}

void SendChunkPool::Push(SharedPtr<SendChunk> chunk)
{
    WRITE_GUARD;
    mSendChunks.push_back(chunk);
}

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

void SendBufferManager::Register(SharedPtr<SendBuffer> sendBuf)
{
    mSendBufs.push_back(std::move(sendBuf));
    WSABUF wsaBuf;
    wsaBuf.buf = reinterpret_cast<CHAR*>(mSendBufs.back()->GetBuffer());
    wsaBuf.len = static_cast<ULONG>(mSendBufs.back()->GetWrittenSize());
    mWsaBufs.push_back(wsaBuf);
}

void SendBufferManager::Clear()
{
    mWsaBufs.clear();
    mSendBufs.clear();
}

void SendBufferManager::Swap(SendBufferManager& other) noexcept
{
    mSendBufs.swap(other.mSendBufs);
    mWsaBufs.swap(other.mWsaBufs);
}
