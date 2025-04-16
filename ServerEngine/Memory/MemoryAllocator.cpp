/*    ServerEngine/Memory/Allocator.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Memory/MemoryPool.h"

void* BaseMemoryAllocator::Alloc(UInt64 size)
{
    return ::malloc(size);
}

void BaseMemoryAllocator::Free(void* memory)
{
    ::free(memory);
}

void* StompMemoryAllocator::Alloc(UInt64 size)
{
    const UInt64 pageCount = (size + kPageSize - 1) / kPageSize;
    const UInt64 offset = pageCount * kPageSize - size;
    const UInt64 base = reinterpret_cast<UInt64>(::VirtualAlloc(nullptr, pageCount * kPageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

    return reinterpret_cast<void*>(base + offset);
}

void StompMemoryAllocator::Free(void* memory)
{
    const UInt64 data = reinterpret_cast<UInt64>(memory);
    const UInt64 base = data - (data % kPageSize);
    ::VirtualFree(reinterpret_cast<void*>(base), 0, MEM_RELEASE);
}

void* MemoryPoolAllocator::Alloc(UInt64 size)
{
    return gMemoryPoolManager->Pop(size);
}

void MemoryPoolAllocator::Free(void* memory)
{
    gMemoryPoolManager->Push(memory);
}
