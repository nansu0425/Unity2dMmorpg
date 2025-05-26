/*    ServerCore/Network/Address.h    */

#pragma once

namespace core
{
    /**
     * NetAddress - 네트워크 주소(IP 및 포트) 관리 클래스
     *
     * 윈도우 소켓 API의 SOCKADDR_IN 구조체를 래핑하여 IP 주소와 포트를
     * 편리하게 관리할 수 있게 해주는 클래스입니다.
     *
     * 주요 기능:
     * - IP 문자열과 포트 번호를 이용한 주소 생성
     * - IP 주소의 문자열 변환 지원
     * - 네트워크/호스트 바이트 오더 자동 변환
     * - 윈도우 소켓 API와의 호환성
     */
    class NetAddress
    {
    public:
        NetAddress() = default;
        NetAddress(SOCKADDR_IN address);
        NetAddress(String16View ip, UInt16 port);

        const SOCKADDR_IN& GetAddress() const { return mAddress; }
        String16            GetIp() const;
        UInt16              GetPort() const { return ::ntohs(mAddress.sin_port); }

    public:
        static IN_ADDR      Ip2Addr(String16View ip);

    private:
        SOCKADDR_IN         mAddress = {};
    };
} // namespace core
