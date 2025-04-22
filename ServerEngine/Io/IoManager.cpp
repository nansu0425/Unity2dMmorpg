/*    ServerEngine/Io/IoManager.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Io/IoManager.h"
#include "ServerEngine/Io/IoEvent.h"

IoManager::IoManager()
{
    mIocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    ASSERT_CRASH(mIocp != nullptr, "CREATE_IOCP_FAILED");
}

IoManager::~IoManager()
{
    if (mIocp != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(mIocp);
        mIocp = INVALID_HANDLE_VALUE;
    }
}

Int64 IoManager::RegisterIoObject(IIoObjectOwner* owner)
{
    if (NULL == ::CreateIoCompletionPort(owner->GetIoObject(), mIocp, reinterpret_cast<ULONG_PTR>(owner), 0))
    {
        return ::GetLastError();
    }

    return 0;
}

Int64 IoManager::DispatchIoEvent(UInt32 timeoutMs)
{
    DWORD numBytes = 0;
    IIoObjectOwner* owner = nullptr;
    IoEvent* event = nullptr;

    if (FALSE == ::GetQueuedCompletionStatus(mIocp, OUT &numBytes, OUT reinterpret_cast<PULONG_PTR>(&owner), OUT reinterpret_cast<LPOVERLAPPED*>(&event), timeoutMs))
    {
        return ::GetLastError();
    }

    ASSERT_CRASH_DEBUG(owner != nullptr, "INVALID_OWNER");
    owner->DispatchIoEvent(event, numBytes);

    return 0;
}

// TEMP
IoManager gIoManager;
