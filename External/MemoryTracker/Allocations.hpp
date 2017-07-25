///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "DllMain.hpp"
#include <new>

#pragma comment(lib, "MemoryTracker.lib")

void* DebugAllocate(size_t size, int framesToSkip = 3)
{
  return AllocateMemory(size,  framesToSkip);
}

void DebugDeallocate(void* memory)
{
  DeallocateMemory(memory);
}

#pragma warning(push)
#pragma warning(disable: 4290)
void* operator new(size_t size) throw (std::bad_alloc) {return DebugAllocate(size);}
void* operator new(size_t size, const std::nothrow_t&) throw () {return DebugAllocate(size);}
void* operator new[](size_t size) throw (std::bad_alloc) {return DebugAllocate(size);}
void* operator new[](size_t size, const std::nothrow_t&) throw () {return DebugAllocate(size);}
void operator delete(void* ptr) throw () {DebugDeallocate(ptr);}
void operator delete(void* ptr, const std::nothrow_t&) throw () {DebugDeallocate(ptr);}
void operator delete[](void* ptr) throw () {DebugDeallocate(ptr);}
void operator delete[](void* ptr, const std::nothrow_t&) throw () {DebugDeallocate(ptr);}
#pragma warning(pop)
