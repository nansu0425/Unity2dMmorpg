/*    ServerCore/Io/Dispatcher.cpp    */

#include "ServerCore/Pch.h"
#include "ServerCore/Io/Dispatcher.h"
#include "ServerCore/Io/Event.h"

namespace core
{
    /**
     * IoEventDispatcher 생성자
     *
     * 비동기 IO 작업을 관리하기 위한 IO 완료 포트(IOCP)를 생성합니다.
     * 생성에 실패할 경우 크래시가 발생하므로, 항상 유효한 IOCP를 보장합니다.
     */
    IoEventDispatcher::IoEventDispatcher()
    {
        mIocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
        ASSERT_CRASH(mIocp != nullptr, "CREATE_IOCP_FAILED");
    }

    /**
     * IoEventDispatcher 소멸자
     *
     * IO 완료 포트에 할당된 리소스를 정리합니다.
     * 유효한 IOCP 핸들이 있다면 안전하게 닫습니다.
     */
    IoEventDispatcher::~IoEventDispatcher()
    {
        if (mIocp != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(mIocp);
            mIocp = INVALID_HANDLE_VALUE;
        }
    }

    /**
     * IO 객체 등록
     *
     * 지정된 IO 객체(Listener나 Session)를 IOCP에 등록합니다.
     * 등록된 객체는 비동기 IO 작업 완료 시 통지를 받을 수 있습니다.
     *
     * @param owner 등록할 IO 객체 소유자
     * @return SUCCESS 성공 시, 오류 코드 실패 시
     */
    Int64 IoEventDispatcher::Register(SharedPtr<IIoObjectOwner> owner)
    {
        if (NULL == ::CreateIoCompletionPort(owner->GetIoObject(), mIocp, 0, 0))
        {
            return ::GetLastError();
        }

        return SUCCESS;
    }

    /**
     * IO 이벤트 디스패치
     *
     * 완료 큐에서 IO 이벤트를 가져와 해당 소유자에게 전달합니다.
     * timeoutMs 동안 대기하며, 이벤트가 없으면 WAIT_TIMEOUT을 반환합니다.
     *
     * 주요 단계:
     * 1. GetQueuedCompletionStatus 호출로 완료된 이벤트 대기
     * 2. 이벤트 수신 시 결과 코드 저장
     * 3. 이벤트 소유자에게 DispatchIoEvent 호출을 통해 통지
     *
     * @param timeoutMs 이벤트 대기 제한 시간(밀리초), INFINITE는 무한 대기
     * @return SUCCESS 정상 처리 시, WAIT_TIMEOUT 제한 시간 초과 시, 기타 오류 코드
     */
    Int64 IoEventDispatcher::Dispatch(UInt32 timeoutMs)
    {
        DWORD numBytes = 0;
        ULONG_PTR completionKey = 0;
        IoEvent* event = nullptr;
        SharedPtr<IIoObjectOwner> owner = nullptr;
        Int64 result = SUCCESS;

        // 입출력 이벤트를 꺼낼 수 있을 때까지 대기
        if (FALSE == ::GetQueuedCompletionStatus(mIocp, OUT & numBytes, OUT & completionKey, OUT reinterpret_cast<LPOVERLAPPED*>(&event), timeoutMs))
        {
            result = ::GetLastError();

            switch (result)
            {
            case WAIT_TIMEOUT: // 대기 시간 초과
                return result;
            default:
                break;
            }

            if (event == nullptr)
            {
                gLogger->Error(TEXT_8("Failed to get queued completion status: {}"), result);
            }
        }

        // 이벤트를 정상적으로 꺼냈으면
        if (event != nullptr)
        {
            // 입출력 결과 저장
            event->result = result;
            // 입출력 이벤트 전달
            owner = event->owner;
            owner->DispatchIoEvent(event, numBytes);
        }

        return SUCCESS;
    }
} // namespace core
