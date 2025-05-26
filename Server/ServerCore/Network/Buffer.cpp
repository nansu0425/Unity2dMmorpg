/*    ServerCore/Network/Buffer.cpp    */

#include "ServerCore/Pch.h"
#include "ServerCore/Network/Buffer.h"

namespace core
{
    /**
     * ReceiveBuffer 생성자
     *
     * 지정된 크기의 네트워크 수신 버퍼를 생성합니다.
     * 실제 할당되는 버퍼 크기는 요청 크기의 kCapacityFactor배입니다.
     *
     * @param size 기본 버퍼 크기
     */
    ReceiveBuffer::ReceiveBuffer(Int64 size)
        : mSize(size)
        , mCapacity(size* kCapcityFactor)
    {
        mBuffer.resize(mCapacity);
    }

    /**
     * ReceiveBuffer 소멸자
     */
    ReceiveBuffer::~ReceiveBuffer()
    {}

    /**
     * 버퍼 정리 및 최적화
     *
     * 버퍼의 상태에 따라 다음 작업을 수행합니다:
     * 1. 모든 데이터를 읽은 경우 - 읽기/쓰기 위치를 0으로 초기화
     * 2. 여유 공간이 부족한 경우 - 읽지 않은 데이터를 버퍼 앞으로 이동
     *
     * 이를 통해 지속적인 버퍼 사용 시 메모리 효율을 유지합니다.
     */
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

    /**
     * 데이터 읽기 처리
     *
     * 버퍼에서 데이터를 읽은 후 읽기 위치를 업데이트합니다.
     * 유효한 데이터보다 더 많은 데이터를 읽으려고 하면 실패합니다.
     *
     * @param numBytes 읽은 바이트 수
     * @return 성공 여부
     */
    Bool ReceiveBuffer::OnRead(Int64 numBytes)
    {
        ASSERT_CRASH_DEBUG(0 <= numBytes, "INVALID_NUM_BYTES");

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

    /**
     * 데이터 쓰기 처리
     *
     * 버퍼에 데이터를 쓴 후 쓰기 위치를 업데이트합니다.
     * 여유 공간보다 더 많은 데이터를 쓰려고 하면 실패합니다.
     *
     * @param numBytes 쓴 바이트 수
     * @return 성공 여부
     */
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

    /**
     * SendBuffer 생성자
     *
     * SendChunk에서 할당받은 메모리 영역으로 송신 버퍼를 초기화합니다.
     *
     * @param owner 이 버퍼를 소유한 청크
     * @param buffer 할당받은 메모리 버퍼 포인터
     * @param allocSize 할당받은 메모리 크기
     */
    SendBuffer::SendBuffer(SharedPtr<SendChunk> owner, Byte* buffer, Int64 allocSize)
        : mBuffer(buffer)
        , mAllocSize(allocSize)
        , mOwner(std::move(owner))
    {}

    /**
     * SendBuffer 소멸자
     */
    SendBuffer::~SendBuffer()
    {}

    /**
     * 데이터 쓰기 완료 처리
     *
     * 버퍼에 데이터 쓰기가 완료된 후 실제 쓰여진 크기를 기록하고
     * 소유 청크에 쓰기 완료를 알립니다.
     *
     * @param writtenSize 실제로 쓰여진 바이트 수
     */
    void SendBuffer::OnWritten(Int64 writtenSize)
    {
        ASSERT_CRASH(writtenSize <= mAllocSize, "BUFFER_OVERFLOW");
        mWrittenSize = writtenSize;
        mOwner->OnWritten(mWrittenSize);
    }

    /**
     * SendBuffer 할당
     *
     * 요청된 크기만큼의 메모리를 할당하여 SendBuffer 객체를 생성합니다.
     * 청크에 여유 공간이 없거나 이미 쓰기 중이면 실패합니다.
     *
     * @param allocSize 할당할 메모리 크기
     * @return 할당된 SendBuffer 객체
     */
    SharedPtr<SendBuffer> SendChunk::Alloc(Int64 allocSize)
    {
        ASSERT_CRASH(allocSize <= GetFreeSize(), "BUFFER_OVERFLOW");
        ASSERT_CRASH(mIsWriting == false, "WRITING_STATE");

        mIsWriting = true;

        return std::make_shared<SendBuffer>(shared_from_this(), AtWritePos(), allocSize);
    }

    /**
     * 쓰기 완료 처리
     *
     * SendBuffer에서 데이터 쓰기가 완료된 후 청크의 상태를 업데이트합니다.
     *
     * @param writtenSize 쓰여진 바이트 수
     */
    void SendChunk::OnWritten(Int64 writtenSize)
    {
        ASSERT_CRASH(mIsWriting == true, "NOT_WRITING_STATE");
        mIsWriting = false;
        mWrittenSize += writtenSize;
    }

    /**
     * 청크 초기화
     *
     * 청크의 상태를 초기화하여 재사용할 수 있게 합니다.
     * 실제 메모리 내용은 지우지 않고 상태 플래그와 크기만 초기화합니다.
     */
    void SendChunk::Clear()
    {
        mIsWriting = false;
        mWrittenSize = 0;
    }

    /**
     * 송신 버퍼 할당
     *
     * 스레드별 SendChunk를 관리하며 요청된 크기의 SendBuffer를 할당합니다.
     * 필요시 새 청크를 풀에서 가져오거나 생성합니다.
     *
     * @param allocSize 할당할 메모리 크기
     * @return 할당된 SendBuffer 객체
     */
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

    /**
     * SendChunk 삭제 처리
     *
     * 사용이 끝난 청크를 풀에 반환하여 재사용할 수 있게 합니다.
     * 스마트 포인터의 삭제자로 사용됩니다.
     *
     * @param chunk 반환할 SendChunk 포인터
     */
    void SendChunkPool::Delete(SendChunk* chunk)
    {
        // 청크를 재사용하기 위해 풀링
        gSendChunkPool->Push(SharedPtr<SendChunk>(chunk, Delete));
    }

    /**
     * 청크 풀에서 청크 가져오기
     *
     * 사용 가능한 청크가 있으면 풀에서 가져오고, 없으면 새로 생성합니다.
     *
     * @return 사용 가능한 SendChunk 객체
     */
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

    /**
     * 청크를 풀에 반환
     *
     * 사용이 끝난 청크를 풀에 반환하여 재사용할 수 있게 합니다.
     *
     * @param chunk 반환할 SendChunk 객체
     */
    void SendChunkPool::Push(SharedPtr<SendChunk> chunk)
    {
        WRITE_GUARD;
        mSendChunks.push_back(chunk);
    }

    /**
     * BufferReader 기본 생성자
     *
     * 비어있는 버퍼 리더를 생성합니다.
     */
    BufferReader::BufferReader()
    {}

    /**
     * BufferReader 초기화 생성자
     *
     * 지정된 버퍼와 크기로 버퍼 리더를 초기화합니다.
     *
     * @param buffer 읽을 버퍼 포인터
     * @param size 버퍼 크기
     * @param pos 시작 위치 (기본값 0)
     */
    BufferReader::BufferReader(Byte* buffer, Int64 size, Int64 pos)
        : mBuffer(buffer)
        , mSize(size)
        , mPos(pos)
    {}

    /**
     * BufferReader 소멸자
     */
    BufferReader::~BufferReader()
    {}

    /**
     * 버퍼 미리보기
     *
     * 현재 위치에서 데이터를 읽되, 읽기 위치는 변경하지 않습니다.
     *
     * @param dest 데이터를 저장할 목적지 버퍼
     * @param size 읽을 바이트 수
     * @return 성공 여부
     */
    Bool BufferReader::Peek(Byte* dest, Int64 size)
    {
        if (GetFreeSize() < size)
        {
            return false;
        }
        ::memcpy(dest, mBuffer + mPos, size);

        return true;
    }

    /**
     * 버퍼 읽기
     *
     * 현재 위치에서 데이터를 읽고 읽기 위치를 업데이트합니다.
     *
     * @param dest 데이터를 저장할 목적지 버퍼
     * @param size 읽을 바이트 수
     * @return 성공 여부
     */
    Bool BufferReader::Read(Byte* dest, Int64 size)
    {
        if (Peek(dest, size) == false)
        {
            return false;
        }
        mPos += size;

        return true;
    }

    /**
     * BufferWriter 기본 생성자
     *
     * 비어있는 버퍼 라이터를 생성합니다.
     */
    BufferWriter::BufferWriter()
    {}

    /**
     * BufferWriter 초기화 생성자
     *
     * 지정된 버퍼와 크기로 버퍼 라이터를 초기화합니다.
     *
     * @param buffer 쓸 버퍼 포인터
     * @param size 버퍼 크기
     * @param pos 시작 위치 (기본값 0)
     */
    BufferWriter::BufferWriter(Byte* buffer, Int64 size, Int64 pos)
        : mBuffer(buffer)
        , mSize(size)
        , mPos(pos)
    {}

    /**
     * BufferWriter 소멸자
     */
    BufferWriter::~BufferWriter()
    {}

    /**
     * 버퍼에 데이터 쓰기
     *
     * 현재 위치에 데이터를 쓰고 쓰기 위치를 업데이트합니다.
     *
     * @param src 쓸 소스 데이터 버퍼
     * @param size 쓸 바이트 수
     * @return 성공 여부
     */
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

    /**
     * SendBuffer 등록
     *
     * 송신 버퍼를 매니저에 등록하고 WSA 버퍼 배열을 업데이트합니다.
     *
     * @param sendBuf 등록할 송신 버퍼
     */
    void SendBufferManager::Register(SharedPtr<SendBuffer> sendBuf)
    {
        mSendBufs.push_back(std::move(sendBuf));
        WSABUF wsaBuf;
        wsaBuf.buf = reinterpret_cast<CHAR*>(mSendBufs.back()->GetBuffer());
        wsaBuf.len = static_cast<ULONG>(mSendBufs.back()->GetWrittenSize());
        mWsaBufs.push_back(wsaBuf);
    }

    /**
     * 버퍼 관리자 초기화
     *
     * 등록된 모든 SendBuffer와 WSABUF 배열을 비웁니다.
     */
    void SendBufferManager::Clear()
    {
        mWsaBufs.clear();
        mSendBufs.clear();
    }

    /**
     * 버퍼 관리자 교체
     *
     * 다른 SendBufferManager와 내용을 교환합니다.
     *
     * @param other 교환할 다른 SendBufferManager
     */
    void SendBufferManager::Swap(SendBufferManager& other) noexcept
    {
        mSendBufs.swap(other.mSendBufs);
        mWsaBufs.swap(other.mWsaBufs);
    }
} // namespace core
