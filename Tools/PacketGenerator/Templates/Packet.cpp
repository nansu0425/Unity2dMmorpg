/*    {{ project_name }}/Network/Protocol/Packet.cpp    */

#include "{{ project_name }}/Pch.h"
#include "{{ project_name }}/Network/Protocol/Packet.h"
#include "{{ project_name }}/Network/Session.h"

/**
 * @brief 모든 핸들러를 유효하지 않은 핸들러로 설정하여 PacketHandlerMap을 구성합니다.
 *
 * 모든 가능한 패킷 ID가 유효하지 않은 핸들러 함수와 연결되도록 핸들러 맵을 초기화하여
 * 처리되지 않은 패킷이 문제를 일으키지 않도록 합니다.
 */
PacketHandlerMap::PacketHandlerMap()
{
    // Invalid 핸들러로 초기화
    for (Int32 i = 0; i < std::numeric_limits<Int16>::max() + 1; ++i)
    {
        mIdToHandler[i] = [this](const Packet& packet) { return Handle_Invalid(packet); };
    }
}

/**
 * @brief 등록되지 않은 패킷 ID에 대한 기본 핸들러입니다.
 *
 * 세션 정보와 유효하지 않은 패킷 ID로 오류를 기록합니다.
 * 등록되지 않은 ID의 패킷이 수신될 때 호출됩니다.
 *
 * @param packet 유효하지 않은 ID를 가진 수신된 패킷
 * @return 항상 처리 실패를 나타내는 false를 반환
 */
Bool PacketHandlerMap::Handle_Invalid(const Packet& packet)
{
    gLogger->Error(TEXT_8("Session[{}]: Invalid packet id: {}"), packet.GetOwner()->GetId(), packet.GetId());
    return false;
}

/**
 * @brief 수신 버퍼에서 여러 패킷을 처리합니다.
 *
 * 버퍼에서 패킷을 하나씩 읽어 각 완전한 패킷을 등록된 핸들러에
 * 전달합니다. 부분 패킷이 발견되면 중지합니다.
 *
 * @param handlers 패킷 처리에 사용할 핸들러 맵
 * @param owner 수신된 패킷을 소유한 세션
 * @param buffer 패킷을 포함하는 원시 데이터 버퍼
 * @param numBytes 버퍼의 총 바이트 수
 * @return 버퍼에서 성공적으로 처리된 바이트 수
 */
Int64 PacketUtils::ProcessPackets(PacketHandlerMap& handlers, SharedPtr<Session> owner, const Byte* buffer, Int64 numBytes)
{
    Int64 processedSize = 0;

    // 처리 가능한 모든 패킷 처리
    while (processedSize < numBytes)
    {
        // 패킷 헤더 크기만큼 수신했는지 확인
        if (numBytes - processedSize < sizeof_64(PacketHeader))
        {
            break;
        }

        // 패킷 헤더를 읽어 패킷 크기와 ID를 확인
        const PacketHeader* header = reinterpret_cast<const PacketHeader*>(buffer + processedSize);
        ASSERT_CRASH_DEBUG(header->size > 0, "INVALID_PACKET_SIZE");

        // 패킷의 일부만 수신한 경우
        if (header->size > numBytes - processedSize)
        {
            break;
        }

        // 패킷 처리
        handlers.HandlePacket(Packet(owner, buffer + processedSize));
        processedSize += header->size;
    }

    return processedSize;
}
