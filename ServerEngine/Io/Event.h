/*    ServerEngine/Io/Event.h    */

#pragma once

#include "ServerEngine/Network/Buffer.h"

class Session;
class IIoObjectOwner;

/**
 * IoEventType - 입출력 이벤트 유형을 나타내는 열거형
 *
 * IOCP 모델에서 처리되는 다양한 비동기 IO 작업의 종류를 식별합니다.
 * 각 이벤트 유형은 특정 IO 작업의 완료를 나타냅니다.
 */
enum class IoEventType : Int64
{
    Connect,    // 연결 요청 완료
    Disconnect, // 연결 해제 완료
    Accept,     // 연결 수락 완료
    Receive,    // 데이터 수신 완료
    Send,       // 데이터 송신 완료
};

/**
 * IoEvent - IOCP 비동기 입출력 작업 기본 구조체
 *
 * Windows OVERLAPPED 구조체를 상속하여 IOCP 작업에 사용되는 기본 클래스입니다.
 * 모든 비동기 IO 이벤트의 공통 속성을 정의하며, 각 이벤트 타입별로 파생 구조체가 있습니다.
 *
 * 주요 멤버:
 * - type: 이벤트 타입 (Connect, Disconnect, Accept, Receive, Send)
 * - owner: 이벤트를 소유하는 객체(보통 Session이나 Listener)에 대한 참조
 * - result: 작업 결과 코드 (SUCCESS 또는 오류 코드)
 */
struct IoEvent
    : public OVERLAPPED
{
    IoEventType                 type;
    SharedPtr<IIoObjectOwner>   owner; // 입출력을 요청한 객체가 입출력 작업이 진행되는 동안 살아있도록 보장한다
    Int64                       result; // 입출력 작업 결과, SUCCESS가 아니면 에러 코드

    void Init() { ::ZeroMemory(this, sizeof(OVERLAPPED)); }

    IoEvent(IoEventType type) : type(type) { Init(); }
};

/**
 * ConnectEvent - 비동기 연결 요청 완료 이벤트
 *
 * ConnectEx API를 사용한 비동기 연결 요청이 완료되었을 때 사용됩니다.
 * 클라이언트 측에서 서버로의 연결 시도 결과를 처리합니다.
 */
struct ConnectEvent
    : public IoEvent
{
    ConnectEvent() : IoEvent(IoEventType::Connect) {}
};

/**
 * DisconnectEvent - 비동기 연결 해제 완료 이벤트
 *
 * DisconnectEx API를 사용한 비동기 연결 해제 요청이 완료되었을 때 사용됩니다.
 * 연결 해제의 원인을 포함하여 세션의 정상적인 종료를 처리합니다.
 */
struct DisconnectEvent
    : public IoEvent
{
    String8     cause;  // 연결 해제 원인

    DisconnectEvent() : IoEvent(IoEventType::Disconnect) {}
};

/**
 * AcceptEvent - 비동기 연결 수락 완료 이벤트
 *
 * AcceptEx API를 사용한 비동기 연결 수락 요청이 완료되었을 때 사용됩니다.
 * 서버 측에서 새로운 클라이언트 연결을 수락하고 세션을 생성하는 데 사용됩니다.
 */
struct AcceptEvent
    : public IoEvent
{
    SharedPtr<Session>  session = nullptr;  // 새로 생성된 세션

    AcceptEvent() : IoEvent(IoEventType::Accept) {}
};

/**
 * ReceiveEvent - 비동기 데이터 수신 완료 이벤트
 *
 * WSARecv API를 사용한 비동기 데이터 수신 요청이 완료되었을 때 사용됩니다.
 * 수신된 데이터 처리를 위해 세션에 통지됩니다.
 */
struct ReceiveEvent
    : public IoEvent
{
    ReceiveEvent() : IoEvent(IoEventType::Receive) {}
};

/**
 * SendEvent - 비동기 데이터 송신 완료 이벤트
 *
 * WSASend API를 사용한 비동기 데이터 송신 요청이 완료되었을 때 사용됩니다.
 * 송신에 사용된 버퍼 관리자를 포함하여 데이터 송신 완료를 처리합니다.
 */
struct SendEvent
    : public IoEvent
{
    SendBufferManager   bufferMgr;  // 송신에 사용된 버퍼 관리자

    SendEvent() : IoEvent(IoEventType::Send) {}
};
