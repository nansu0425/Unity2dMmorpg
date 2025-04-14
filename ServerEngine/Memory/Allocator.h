/*    ServerEngine/Memory/Allocator.h    */

#pragma once

class BaseAllocator
{
public:
    static void*    Allocate(UInt64 size);
    static void     Free(void* memory);
};
