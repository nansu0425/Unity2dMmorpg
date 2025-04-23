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
