/*    ServerEngine/Memory/Allocator.h    */

#pragma once

void* AllocateMemory(UInt64 size);
void FreeMemory(void* memory);

template<typename T, typename ...Args>
T* CreateObject(Args&&... args);
template<typename T>
void DestroyObject(T* object);

#include "ServerEngine/Memory/Allocator_Impl.h"
