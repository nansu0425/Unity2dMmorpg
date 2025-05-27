/*    Core/Io/Dispatcher.h    */

#pragma once

namespace core
{
    struct IoEvent;

    /**
     * IIoObjectOwner - IO 객체 소유자 인터페이스
     *
     * IOCP 모델에서 비동기 IO 이벤트를 처리할 수 있는 객체를 위한 인터페이스입니다.
     * Listener와 Session 클래스가 이 인터페이스를 구현하여 IO 이벤트를 수신합니다.
     *
     * 주요 책임:
     * - IO 작업을 위한 핸들(주로 소켓) 제공
     * - IO 완료 이벤트 처리 방법 구현
     *
     * 사용 패턴:
     * - IoEventDispatcher에 등록하여 IOCP 이벤트 수신
     * - 이벤트가 발생하면 DispatchIoEvent 메서드로 통지 받음
     */
    class IIoObjectOwner
        : public std::enable_shared_from_this<IIoObjectOwner>
    {
    public:
        virtual HANDLE  GetIoObject() = 0;
        virtual void    DispatchIoEvent(IoEvent* event, Int64 numBytes = 0) = 0;
    };

    /**
     * IoEventDispatcher - IO 완료 포트(IOCP) 관리 클래스
     *
     * Windows의 IO 완료 포트(IOCP)를 래핑한 클래스로, 비동기 IO 작업의 완료를
     * 효율적으로 감지하고 처리합니다. 네트워크 서버의 핵심 비동기 이벤트 루프를 구현합니다.
     *
     * 주요 기능:
     * - IO 완료 포트 생성 및 관리
     * - IO 객체(소켓 등)의 등록
     * - 완료된 IO 이벤트 감지 및 디스패치
     * - 비동기 IO 모델의 스케줄링 최적화
     *
     * 사용 패턴:
     * 1. IoEventDispatcher 인스턴스 생성
     * 2. 다양한 IO 객체(Listener, Session) 등록
     * 3. 워커 스레드에서 Dispatch 메서드 반복 호출
     * 4. 완료된 이벤트가 감지되면 해당 소유자의 DispatchIoEvent 메서드 호출
     */
    class IoEventDispatcher
    {
    public:
        IoEventDispatcher();
        ~IoEventDispatcher();

        HANDLE          GetIocp() const { return mIocp; }

        Int64           Register(SharedPtr<IIoObjectOwner> owner);
        Int64           Dispatch(UInt32 timeoutMs = INFINITE);

    private:
        HANDLE          mIocp = INVALID_HANDLE_VALUE;
    };
} // namespace core
