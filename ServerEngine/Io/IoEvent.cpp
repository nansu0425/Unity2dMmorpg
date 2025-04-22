/*    ServerEngine/Io/IoEvent.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Io/IoEvent.h"

IoEvent::IoEvent(IoEventType type)
    : mType(type)
{
    Init();
}

void IoEvent::Init()
{
    ::ZeroMemory(this, sizeof(OVERLAPPED));
}
