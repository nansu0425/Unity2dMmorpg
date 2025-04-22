/*    ServerEngine/Io/IoManager.h    */

#pragma once

class IIoObjectOwner
{
public:
    virtual HANDLE GetIoObject() = 0;
    virtual void   DispatchIoEvent(class IoEvent* event, Int64 numBytes = 0) = 0;
};

class IoManager
{
public:
    IoManager();
    ~IoManager();

    HANDLE  GetIocp() const { return mIocp; }

    Int64   RegisterIoObject(class IIoObjectOwner* owner);
    Int64   DispatchIoEvent(UInt32 timeoutMs = INFINITE);

private:
    HANDLE  mIocp = INVALID_HANDLE_VALUE;
};

// TEMP
extern IoManager gIoManager;
