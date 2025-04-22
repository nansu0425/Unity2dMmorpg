/*    ServerEngine/Io/Dispatcher.h    */

#pragma once

struct IoEvent;

class IIoObjectOwner
    : public std::enable_shared_from_this<IIoObjectOwner>
{
public:
    virtual HANDLE  GetIoObject() = 0;
    virtual void    DispatchIoEvent(IoEvent* event, Int64 numBytes = 0) = 0;
};

class IoEventDispatcher
{
public:
    IoEventDispatcher();
    ~IoEventDispatcher();

    HANDLE  GetIocp() const { return mIocp; }

    Int64   Register(SharedPtr<IIoObjectOwner> owner);
    Int64   Dispatch(UInt32 timeoutMs = INFINITE);

private:
    HANDLE  mIocp = INVALID_HANDLE_VALUE;
};
