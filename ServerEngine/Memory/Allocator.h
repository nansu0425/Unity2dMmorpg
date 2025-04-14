/*    ServerEngine/Memory/Allocator.h    */

#pragma once

class BaseAllocator
{
public:
    static void*    Alloc(UInt64 size);
    static void     Free(void* memory);
};
