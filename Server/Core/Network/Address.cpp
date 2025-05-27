/*    Core/Network/Address.cpp    */

#include "Core/Pch.h"
#include "Core/Network/Address.h"

namespace core
{
    /**
     * 기존 SOCKADDR_IN 구조체로부터 NetAddress 객체 생성
     *
     * 윈도우 소켓 API에서 사용하는 SOCKADDR_IN 구조체를
     * NetAddress 객체로 변환할 때 사용합니다.
     *
     * @param address 변환할 SOCKADDR_IN 구조체
     */
    NetAddress::NetAddress(SOCKADDR_IN address)
        : mAddress(address)
    {}

    /**
     * IP 주소와 포트로부터 NetAddress 객체 생성
     *
     * 문자열 형태의 IP 주소와 정수형 포트 번호를 받아
     * 소켓에서 사용 가능한 네트워크 주소 객체로 변환합니다.
     *
     * @param ip IP 주소 문자열 (IPv4 형식, 예: "127.0.0.1")
     * @param port 포트 번호 (1-65535)
     */
    NetAddress::NetAddress(String16View ip, UInt16 port)
    {
        ::memset(&mAddress, 0, sizeof(mAddress));
        mAddress.sin_family = AF_INET;
        mAddress.sin_addr = Ip2Addr(ip);
        mAddress.sin_port = ::htons(port);
    }

    /**
     * IP 주소를 문자열 형태로 반환
     *
     * 내부 SOCKADDR_IN 구조체에 저장된 IP 주소를
     * 사람이 읽을 수 있는 문자열 형태로 변환합니다.
     *
     * @return IPv4 형식의 IP 주소 문자열
     */
    String16 NetAddress::GetIp() const
    {
        Char16 buffer[100] = {};
        ::InetNtop(AF_INET, &mAddress.sin_addr, OUT buffer, countof_64(buffer));

        return String16(buffer);
    }

    /**
     * 문자열 IP 주소를 IN_ADDR 구조체로 변환
     *
     * IPv4 형식의 문자열 주소를 윈도우 소켓 API에서 사용하는
     * IN_ADDR 구조체로 변환합니다.
     *
     * @param ip 변환할 IPv4 형식 IP 주소 문자열
     * @return 변환된 IN_ADDR 구조체
     */
    IN_ADDR NetAddress::Ip2Addr(String16View ip)
    {
        IN_ADDR addr;
        ::InetPton(AF_INET, ip.data(), OUT & addr);

        return addr;
    }
} // namespace core
