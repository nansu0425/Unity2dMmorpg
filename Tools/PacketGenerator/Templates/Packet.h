/*    {{ project_name }}/Network/Protocol/Packet.h    */

#pragma once

{%- for prefix, payloads in parser.payload_dict.items() %}
#include "{{ project_name }}/Network/Protocol/{{ prefix }}_Payload.pb.h"
{%- endfor %}

class Session;

/**
 * @enum PacketId
 * @brief 전송되는 네트워크 패킷의 유형을 식별합니다.
 * 
 * 시스템의 모든 유효한 패킷 유형을 고유 ID로 열거합니다.
 * 수신된 패킷을 역직렬화하고 처리하는 방법을 결정하는 데 사용됩니다.
 */
enum class PacketId : Int16
{
    Invalid = 0,
    {%- for prefix, payloads in parser.payload_dict.items() %}
    {%- for payload in payloads %}
    {{ payload.name }} = {{ payload.pkt_id }},
    {%- endfor %}
    {%- endfor %}
};

/**
 * @struct PacketHeader
 * @brief 모든 네트워크 패킷의 헤더 정보입니다.
 * 
 * 패킷 처리를 위한 메타데이터(전체 패킷 크기 및 패킷 식별자)를 포함합니다.
 * 모든 패킷 데이터의 시작 부분에 위치합니다.
 */
#pragma pack(push, 1)
struct PacketHeader
{
    Int16       size = 0; // 헤더까지 포함한 패킷의 전체 크기
    PacketId    id = PacketId::Invalid; // 패킷 ID
};
#pragma pack(pop)

/**
 * @class Packet
 * @brief 헤더와 페이로드가 있는 수신된 네트워크 패킷을 캡슐화합니다.
 * 
 * 패킷 데이터에 대한 구조화된 접근을 제공하고 패킷과 
 * 해당 패킷을 소유한 세션 간의 관계를 관리합니다.
 */
class Packet
{
public:
    /**
     * @brief 원시 버퍼 데이터에서 패킷을 구성합니다.
     * @param owner 이 패킷을 수신한 세션
     * @param buffer 원시 패킷 데이터 버퍼
     */
    explicit Packet(SharedPtr<Session> owner, const Byte* buffer)
        : mOwner(std::move(owner))
        , mHeader(reinterpret_cast<const PacketHeader*>(buffer))
        , mPayload(buffer + sizeof_16(PacketHeader))
    {}

    SharedPtr<Session>      GetOwner() const { return mOwner; }
    const PacketHeader*     GetHeader() const { return mHeader; }
    Int16                   GetSize() const { return mHeader->size; }
    Int16                   GetId() const { return static_cast_16(mHeader->id); }
    const Byte*             GetPayload() const { return mPayload; }

private:
    SharedPtr<Session>      mOwner;
    const PacketHeader*     mHeader;
    const Byte*             mPayload;
};

/**
 * @class PacketHandlerMap
 * @brief 패킷 ID를 적절한 핸들러 함수에 매핑합니다.
 * 
 * 패킷 ID를 기반으로 들어오는 패킷을 지정된 핸들러 함수에 
 * 등록하고 라우팅하는 패킷 처리 시스템의 기본 클래스입니다.
 */
class PacketHandlerMap
{
public:
    using PacketHandler     = Function<Bool(const Packet&)>;

public:
    /**
     * @brief 등록된 핸들러를 호출하여 패킷을 처리합니다.
     * @param packet 처리할 패킷
     * @return 패킷이 성공적으로 처리되면 true, 그렇지 않으면 false
     */
    Bool                HandlePacket(const Packet& packet) { return mIdToHandler[packet.GetId()](packet); }

protected:
    /**
     * @brief 기본 유효하지 않은 핸들러로 핸들러 맵을 초기화합니다.
     */
    PacketHandlerMap();

    /**
     * @brief 특정 패킷 ID에 대한 핸들러 함수를 등록합니다.
     * @param handler 지정된 패킷 ID에 대해 호출할 함수
     * @param id 핸들러와 연결할 패킷 ID
     */
    void                RegisterHandler(PacketHandler handler, PacketId id) { mIdToHandler[static_cast<Int16>(id)] = std::move(handler); }
    
    /**
     * @brief 파생 클래스가 구현할 모든 패킷 핸들러를 등록합니다.
     */
    virtual void        RegisterAllHandlers() = 0;

    /**
     * @brief 프로토콜 버퍼 페이로드를 역직렬화하고 처리합니다.
     * @tparam TPayload 프로토콜 버퍼 메시지 유형
     * @tparam TPayloadHandler 페이로드를 처리하는 핸들러 함수 유형
     * @param handler 역직렬화된 페이로드를 처리하는 함수
     * @param packet 페이로드를 포함하는 원시 패킷
     * @return 페이로드가 성공적으로 처리되면 true, 그렇지 않으면 false
     */
    template<typename TPayload, typename TPayloadHandler>
    Bool                HandlePayload(TPayloadHandler handler, const Packet& packet)
    {
        TPayload payload;
        if (!payload.ParseFromArray(packet.GetPayload(), packet.GetSize() - sizeof_16(PacketHeader)))
        {
            return false;
        }

        // 페이로드 처리
        return handler(packet.GetOwner(), payload);
    }

    /**
     * @brief 유효하지 않은 패킷 ID에 대한 기본 핸들러
     * @param packet 유효하지 않은 패킷
     * @return 항상 false를 반환
     */
    static Bool         Handle_Invalid(const Packet& packet);

private:
    PacketHandler       mIdToHandler[std::numeric_limits<Int16>::max() + 1];
};

/**
 * @class PacketUtils
 * @brief 패킷 생성 및 처리를 위한 유틸리티 함수들입니다.
 * 
 * 프로토콜 버퍼 메시지에서 송신 버퍼를 생성하고
 * 수신 버퍼에서 여러 패킷을 처리하는 등의 
 * 공통 패킷 작업을 위한 정적 메서드를 제공합니다.
 */
class PacketUtils
{
public:
    /**
     * @brief 직렬화된 패킷이 포함된 송신 버퍼를 생성합니다.
     * @tparam TPayload 직렬화할 프로토콜 버퍼 메시지 유형
     * @param payload 직렬화할 프로토콜 버퍼 메시지
     * @param packetId 패킷에 할당할 ID
     * @return 직렬화된 패킷이 포함된 SendBuffer
     */
    template<typename TPayload>
    static SharedPtr<SendBuffer>    MakePacketBuffer(const TPayload& payload, PacketId packetId)
    {
        const Int16 payloadSize = static_cast<Int16>(payload.ByteSizeLong());
        const Int16 packetSize = sizeof_16(PacketHeader) + payloadSize;
        SharedPtr<SendBuffer> sendBuf = gSendChunkPool->Alloc(packetSize);

        // 헤더 설정
        PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuf->GetBuffer());
        header->size = packetSize;
        header->id = packetId;

        // 페이로드 직렬화
        ASSERT_CRASH(payload.SerializeToArray(header + 1, payloadSize), "SERIALIZE_TO_ARRAY_FAILED");
        sendBuf->OnWritten(packetSize);

        return sendBuf;
    }

    /**
     * @brief 수신 버퍼에서 여러 패킷을 처리합니다.
     * @param handlers 패킷 처리에 사용할 PacketHandlerMap
     * @param owner 패킷을 소유한 세션
     * @param buffer 패킷 데이터가 포함된 원시 버퍼
     * @param numBytes 버퍼의 총 바이트 수
     * @return 성공적으로 처리된 바이트 수
     */
    static Int64                    ProcessPackets(PacketHandlerMap& handlers, SharedPtr<Session> owner, const Byte* buffer, Int64 numBytes);
};
