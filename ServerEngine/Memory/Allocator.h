/*    ServerEngine/Memory/Allocator.h    */

#pragma once

class BaseAllocator
{
public:
    static void*    Alloc(UInt64 size);
    static void     Free(void* memory);
};

/*
 * 할당된 메모리     : [|   페이지|   ][    페이지    ]   ...   [    페이지    ]|
 * 데이터 위치       : [|       [|                  데이터                   ]|
 * 주소             :  |base    |base + offset                              |base + size
 */
class StompAllocator
{
public:
    static void*    Alloc(UInt64 size);
    static void     Free(void* memory);

private:
    static constexpr UInt64     kPageSize = 4096;
};
