/*    ServerEngine/Network/Buffer.h    */

#pragma once

class SendMessageBuilder;

class ReceiveBuffer
{
public:
    ReceiveBuffer(Int64 size);
    ~ReceiveBuffer();

public:
    void            Clear();
    Bool            OnRead(Int64 numBytes);
    Bool            OnWritten(Int64 numBytes);

    Byte*           AtReadPos() { return mBuffer.data() + mReadPos; }
    Byte*           AtWritePos() { return mBuffer.data() + mWritePos; }
    Int64           GetDataSize() const { return mWritePos - mReadPos; }
    Int64           GetFreeSize() const { return mCapacity - mWritePos; }

private:
    enum Constants : Int64
    {
        kCapcityFactor = 10, // 버퍼 크기에 이 값을 곱하여 실제 용량 설정
    };

private:
    Int64           mSize = 0;
    Int64           mCapacity = 0; // 버퍼의 실제 용량
    Int64           mReadPos = 0;
    Int64           mWritePos = 0;
    Vector<Byte>    mBuffer;
};

class SendBuffers
{
public:
    void        PushMessage(SharedPtr<SendMessageBuilder> message);
    void        Clear();
    void        Swap(SendBuffers& buffers) noexcept;

public:
    WSABUF*     GetWsaBuffers() { return mBuffers.data(); }
    Int64       GetWsaBufferCount() const { return mBuffers.size(); }
    Bool        IsEmpty() const { return mBuffers.empty(); }

private:
    Vector<WSABUF>                          mBuffers;
    Vector<SharedPtr<SendMessageBuilder>>   mMessages;
};

//class SendBuffer
//    : public std::enable_shared_from_this<SendBuffer>
//{
//public:
//    SendBuffer(SharedPtr<SendBufferChunk> owner, Byte* buffer, Int64 allocSize);
//    ~SendBuffer();
//
//    void        Close(Int64 writtenSize);
//
//public:
//    Byte*       GetBuffer() { return mBuffer; }
//    Int64       GetAllocSize() const { return mAllocSize; }
//    Int64       GetWrittenSize() const { return mWrittenSize; }
//
//private:
//    Byte*                       mBuffer; // owner 청크의 일부 영역을 할당받아 버퍼로 사용
//    Int64                       mAllocSize = 0;
//    Int64                       mWrittenSize = 0;
//    SharedPtr<SendBufferChunk>  mOwner;
//};
//
//class SendBufferChunk
//    : public std::enable_shared_from_this<SendBufferChunk>
//{
//public:
//    SendBufferChunk();
//    ~SendBufferChunk();
//
//    SharedPtr<SendBuffer>   Open(Int64 allocSize);
//    void                    Close(Int64 writtenSize);
//    void                    Clear();
//
//    bool                    IsOpen() const { return mIsOpen; }
//    Int64                   GetFreeSize() const { return kChunkSize - mWritePos; }
//    Byte*                   AtWritePos() { return mBuffer + mWritePos; }
//
//private:
//    enum Constants : Int64
//    {
//        kChunkSize  = 0x0001'0000,
//    };;
//
//private:
//    Byte    mBuffer[kChunkSize] = {};
//    Bool    mIsOpen = false;
//    Int64   mWritePos = 0;
//};
//
//class SendBufferManager
//{
//public:
//    SharedPtr<SendBuffer>       Open(Int64 allocSize);
//
//private:
//    SharedPtr<SendBufferChunk>  Pop();
//    void                        Push(SharedPtr<SendBufferChunk> chunk);
//
//    static void                 PushGlobal(SendBufferChunk* chunk);
//
//private:
//    RW_LOCK;
//    Vector<SharedPtr<SendBufferChunk>>  mSendChunks;
//};

class BufferReader
{
public:
    BufferReader();
    BufferReader(Byte* buffer, Int64 size, Int64 pos = 0);
    ~BufferReader();

    Bool    Peek(Byte* dest, Int64 size);
    template<typename T>
    Bool    Peek(T* dest) { return Peek(reinterpret_cast<Byte*>(dest), SIZE_64(T)); }

    Bool    Read(Byte* dest, Int64 size);
    template<typename T>
    Bool    Read(T* dest) { return Read(reinterpret_cast<Byte*>(dest), SIZE_64(T)); }

    template<typename T>
    BufferReader& operator>>(T& dest)
    {
        ASSERT_CRASH(mPos + SIZE_64(T) <= mSize, "BUFFER_OVERFLOW");
        dest = *reinterpret_cast<T*>(mBuffer + mPos);
        mPos += SIZE_64(T);

        return *this;
    }

public:
    Byte*   GetBuffer() { return mBuffer; }
    Int64   GetSize() const { return mSize; }
    Int64   GetReadSize() const { return mPos; }
    Int64   GetFreeSize() const { return mSize - mPos; }

private:
    Byte*   mBuffer = nullptr;
    Int64   mSize = 0;
    Int64   mPos = 0;
};

class BufferWriter
{
public:
    BufferWriter();
    BufferWriter(Byte* buffer, Int64 size, Int64 pos = 0);
    ~BufferWriter();

    Bool    Write(const Byte* src, Int64 size);
    template<typename T>
    Bool    Write(const T* src) { return Write(reinterpret_cast<const Byte*>(src), SIZE_64(T)); }

    template<typename T>
    T* Reserve()
    {
        if (GetFreeSize() < mSize)
        {
            return nullptr;
        }
        T* reserved = reinterpret_cast<T*>(mBuffer + mPos);
        mPos += SIZE_64(T);

        return reserved;
    }

    template<typename T>
    BufferWriter& operator<<(T&& src)
    {
        ASSERT_CRASH(mPos + SIZE_64(T) <= mSize, "BUFFER_OVERFLOW");
        using SrcType = std::remove_reference_t<T>;
        *reinterpret_cast<SrcType*>(mBuffer + mPos) = std::forward<SrcType>(src);
        mPos += SIZE_64(T);

        return *this;
    }

public:
    Byte*   GetBuffer() { return mBuffer; }
    Int64   GetSize() const { return mSize; }
    Int64   GetWrittenSize() const { return mPos; }
    Int64   GetFreeSize() const { return mSize - mPos; }

private:
    Byte*   mBuffer = nullptr;
    Int64   mSize = 0;
    Int64   mPos = 0;
};
