/*    ServerEngine/Network/Dispatcher.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Network/Dispatcher.h"
#include "ServerEngine/Network/Event.h"

IoEventDispatcher::IoEventDispatcher()
{
    mIocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    ASSERT_CRASH(mIocp != nullptr, "CREATE_IOCP_FAILED");
}

IoEventDispatcher::~IoEventDispatcher()
{
    if (mIocp != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(mIocp);
        mIocp = INVALID_HANDLE_VALUE;
    }
}

Int64 IoEventDispatcher::Register(SharedPtr<IIoObjectOwner> owner)
{
    if (NULL == ::CreateIoCompletionPort(owner->GetIoObject(), mIocp, 0, 0))
    {
        return ::GetLastError();
    }

    return SUCCESS;
}

Int64 IoEventDispatcher::Dispatch(UInt32 timeoutMs)
{
    DWORD numBytes = 0;
    ULONG_PTR completionKey = 0;
    IoEvent* event = nullptr;
    SharedPtr<IIoObjectOwner> owner = nullptr;
    Int64 result = SUCCESS;

    // 입출력 이벤트를 꺼낼 수 있을 때까지 대기
    if (FALSE == ::GetQueuedCompletionStatus(mIocp, OUT &numBytes, OUT &completionKey, OUT reinterpret_cast<LPOVERLAPPED*>(&event), timeoutMs))
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
            gLogger->Error(TEXT_16("Failed to get queued completion status: {}"), result);
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
