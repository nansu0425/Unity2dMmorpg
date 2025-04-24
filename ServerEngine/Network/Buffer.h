/*    ServerEngine/Network/Buffer.h    */

#pragma once

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

class SendBuffer
    : public std::enable_shared_from_this<SendBuffer>
{
public:
    SendBuffer(Int64 size);
    ~SendBuffer();

    void            CopyData(const Byte* buffer, Int64 numBytes);

    Byte*           GetBuffer() { return mBuffer.data(); }
    Int64           GetDataSize() const { return mDataSize; }
    Int64           GetCapacity() const { return mBuffer.size(); }

private:
    Vector<Byte>    mBuffer;
    Int64           mDataSize = 0;
};
