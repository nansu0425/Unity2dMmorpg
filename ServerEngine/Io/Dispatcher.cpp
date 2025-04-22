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

    if (FALSE == ::GetQueuedCompletionStatus(mIocp, OUT &numBytes, OUT &completionKey, OUT reinterpret_cast<LPOVERLAPPED*>(&event), timeoutMs))
    {
        return ::GetLastError();
    }

    SharedPtr<IIoObjectOwner> owner = event->owner;
    owner->DispatchIoEvent(event, numBytes);

    return SUCCESS;
}
