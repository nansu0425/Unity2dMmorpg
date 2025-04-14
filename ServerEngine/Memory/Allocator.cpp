/*    ServerEngine/Memory/Allocator.cpp    */

#include "ServerEngine/Pch.h"

void* BaseAllocator::Alloc(UInt64 size)
{
    return ::malloc(size);
}

void BaseAllocator::Free(void* memory)
{
    ::free(memory);
}
