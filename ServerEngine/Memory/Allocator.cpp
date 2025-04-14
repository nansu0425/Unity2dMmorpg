/*    ServerEngine/Memory/Allocator.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Memory/Allocator.h"

void* AllocateMemory(UInt64 size)
{
    return ::malloc(size);
}

void FreeMemory(void* memory)
{
    ::free(memory);
}
