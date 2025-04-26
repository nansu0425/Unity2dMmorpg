/*    ServerEngine/Io/Dispatcher.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Io/Dispatcher.h"
#include "ServerEngine/Io/Event.h"

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
        gLogger->Error(TEXT_16("System error code: {}"), ::GetLastError()); 
    }

    // 입출력 이벤트 전달
    owner = event->owner;
    owner->DispatchIoEvent(event, numBytes);

    return result;
}
