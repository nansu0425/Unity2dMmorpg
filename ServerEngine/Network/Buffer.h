/*    ServerEngine/Network/Buffer.h    */

#pragma once

/**
 * ReceiveBuffer - 네트워크 수신 데이터 관리 클래스
 *
 * 네트워크로부터 데이터를 수신하고 처리하기 위한 순환 버퍼입니다.
 * 읽기/쓰기 위치를 별도로 관리하여 데이터 처리 효율을 높입니다.
 *
 * 주요 기능:
 * - 데이터 읽기/쓰기 위치 별도 관리
 * - 여유 공간이 일정 수준 미만일 때만 읽지 않은 데이터를 버퍼 앞으로 이동
 * - 가변 크기 버퍼 지원 (실제 용량은 kCapcityFactor를 곱한 값으로 설정)
 * - 읽기/쓰기 연산 검증 기능
 */
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
    static constexpr Int64      kCapcityFactor = 16;

private:
    Int64           mSize = 0;
    Int64           mCapacity = 0; // 버퍼의 실제 용량
    Int64           mReadPos = 0;
    Int64           mWritePos = 0;
    Vector<Byte>    mBuffer;
};

class SendChunk;

/**
 * SendBuffer - 네트워크 송신 버퍼 클래스
 *
 * 네트워크로 데이터를 보내기 위한 버퍼를 관리합니다.
 * SendChunk로부터 메모리를 할당받아 사용하며, 실제 쓰기를 한 크기를 추적합니다.
 *
 * 주요 기능:
 * - SendChunk에서 할당받은 메모리 영역 관리
 * - 실제 쓰기 크기 추적
 * - 소유 SendChunk에 쓰기 완료 알림
 */
class SendBuffer
    : public std::enable_shared_from_this<SendBuffer>
{
public:
    SendBuffer(SharedPtr<SendChunk> owner, Byte* buffer, Int64 allocSize);
    ~SendBuffer();

    void        OnWritten(Int64 writtenSize);

public:
    Byte*       GetBuffer() { return mBuffer; }
    Int64       GetAllocSize() const { return mAllocSize; }
    Int64       GetWrittenSize() const { return mWrittenSize; }

private:
    Byte*                   mBuffer; // owner 청크의 일부 영역을 할당받아 버퍼로 사용
    Int64                   mAllocSize = 0;
    Int64                   mWrittenSize = 0;
    SharedPtr<SendChunk>    mOwner;
};

/**
 * SendBufferManager - 다수의 송신 버퍼 관리 클래스
 *
 * 다중 SendBuffer 객체들을 그룹화하여 효율적인 일괄 전송을 지원합니다.
 * Windows WSABUF 구조체와의 연동을 통해 WinSock API 호출에 사용됩니다.
 *
 * 주요 기능:
 * - 다중 SendBuffer 등록 및 관리
 * - WSA API와 호환되는 버퍼 배열 제공
 * - 버퍼 그룹 교체 및 초기화 지원
 */
class SendBufferManager
{
public:
    void        Register(SharedPtr<SendBuffer> sendBuf);
    void        Clear();
    void        Swap(SendBufferManager& other) noexcept;

public:
    WSABUF*     GetWsaBuffers() { return mWsaBufs.data(); }
    Int64       GetWsaBufferCount() const { return mWsaBufs.size(); }
    Bool        IsEmpty() const { return mSendBufs.empty(); }

private:
    Vector<SharedPtr<SendBuffer>>   mSendBufs;
    Vector<WSABUF>                  mWsaBufs;
};

/**
 * SendChunk - 송신 버퍼 메모리 청크 관리 클래스
 *
 * 대용량 메모리 청크를 관리하고 이로부터 SendBuffer에 메모리를 할당합니다.
 * 메모리 조각화를 방지하고 효율적인 할당을 위해 고정 크기 버퍼를 사용합니다.
 *
 * 주요 기능:
 * - 고정 크기 메모리 청크 관리
 * - SendBuffer 객체에 메모리 영역 할당
 * - 쓰기 상태 추적 및 중복 할당 방지
 * - 할당된 영역 재사용을 위한 초기화
 */
class SendChunk
    : public std::enable_shared_from_this<SendChunk>
{
public:
    SharedPtr<SendBuffer>       Alloc(Int64 allocSize);
    void                        OnWritten(Int64 writtenSize);
    void                        Clear();

    bool                        IsWriting() const { return mIsWriting; }
    Int64                       GetFreeSize() const { return kBufferSize - mWrittenSize; }
    Byte*                       AtWritePos() { return mBuffer + mWrittenSize; }

private:
    static constexpr Int64      kBufferSize = 0x0001'0000; // 64KB
    
private:
    Byte        mBuffer[kBufferSize] = {};
    Bool        mIsWriting = false;
    Int64       mWrittenSize = 0;
};

/**
 * SendChunkPool - SendChunk 객체 풀 관리 클래스
 *
 * 다수의 SendChunk 객체를 효율적으로 재사용하기 위한 객체 풀입니다.
 * 스레드별 청크 관리 및 스레드 안전한 풀링 로직을 제공합니다.
 *
 * 주요 기능:
 * - 스레드별 SendChunk 할당 및 관리
 * - 메모리 할당 요청에 따른 적절한 청크 선택
 * - 스레드 안전한 청크 재사용 메커니즘
 * - 사용 완료된 청크의 회수 및 풀링
 */
class SendChunkPool
{
public:
    SharedPtr<SendBuffer>   Alloc(Int64 allocSize);
    static void             Delete(SendChunk* chunk);

private:
    SharedPtr<SendChunk>    Pop();
    void                    Push(SharedPtr<SendChunk> chunk);


private:
    RW_LOCK;
    Vector<SharedPtr<SendChunk>>    mSendChunks;
};

/**
 * BufferReader - 바이트 버퍼 읽기 유틸리티 클래스
 *
 * 바이트 배열로부터 다양한 타입의 데이터를 읽기 위한 유틸리티 클래스입니다.
 * 위치 추적 및 바이트 단위/타입 단위 읽기 연산을 지원합니다.
 *
 * 주요 기능:
 * - 현재 읽기 위치 추적
 * - 바이트 단위 및 타입 단위 읽기 연산
 * - 읽기 전 미리보기(Peek) 기능
 * - 스트림 방식의 연산자 오버로딩
 */
class BufferReader
{
public:
    BufferReader();
    BufferReader(Byte* buffer, Int64 size, Int64 pos = 0);
    ~BufferReader();

    Bool    Peek(Byte* dest, Int64 size);
    template<typename T>
    Bool    Peek(T* dest) { return Peek(reinterpret_cast<Byte*>(dest), sizeof_64(T)); }

    Bool    Read(Byte* dest, Int64 size);
    template<typename T>
    Bool    Read(T* dest) { return Read(reinterpret_cast<Byte*>(dest), sizeof_64(T)); }

    template<typename T>
    BufferReader& operator>>(T& dest)
    {
        ASSERT_CRASH(mPos + sizeof_64(T) <= mSize, "BUFFER_OVERFLOW");
        dest = *reinterpret_cast<T*>(mBuffer + mPos);
        mPos += sizeof_64(T);

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

/**
 * BufferWriter - 바이트 버퍼 쓰기 유틸리티 클래스
 *
 * 바이트 배열에 다양한 타입의 데이터를 기록하기 위한 유틸리티 클래스입니다.
 * 위치 추적 및 바이트 단위/타입 단위 쓰기 연산을 지원합니다.
 *
 * 주요 기능:
 * - 현재 쓰기 위치 추적
 * - 바이트 단위 및 타입 단위 쓰기 연산
 * - 메모리 영역 미리 예약(Reserve) 기능
 * - 스트림 방식의 연산자 오버로딩
 */
class BufferWriter
{
public:
    BufferWriter();
    BufferWriter(Byte* buffer, Int64 size, Int64 pos = 0);
    ~BufferWriter();

    Bool    Write(const Byte* src, Int64 size);
    template<typename T>
    Bool    Write(const T* src) { return Write(reinterpret_cast<const Byte*>(src), sizeof_64(T)); }

    template<typename T>
    T* Reserve()
    {
        if (GetFreeSize() < mSize)
        {
            return nullptr;
        }
        T* reserved = reinterpret_cast<T*>(mBuffer + mPos);
        mPos += sizeof_64(T);

        return reserved;
    }

    template<typename T>
    BufferWriter& operator<<(T&& src)
    {
        ASSERT_CRASH(mPos + sizeof_64(T) <= mSize, "BUFFER_OVERFLOW");
        using SrcType = std::remove_reference_t<T>;
        *reinterpret_cast<SrcType*>(mBuffer + mPos) = std::forward<SrcType>(src);
        mPos += sizeof_64(T);

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
