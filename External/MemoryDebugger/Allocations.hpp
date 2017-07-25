///////////////////////////////////////////////////////////////////////////////
/// Wrapper around the memory debugger.
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "DllMain.hpp"
#include <new>

#pragma comment(lib, "MemoryDebugger.lib")

void* DebugAllocate(size_t size, int allocationType, int framesToSkip = 4)
{
  if(IsDebuggerActive() == 0)
    return malloc(size);
  return AllocateMemory(size, allocationType, framesToSkip);
}

void DebugDeallocate(void* memory, int allocationType)
{
  int success = DeallocateMemory(memory, allocationType);
  if(success == false)
    free(memory);
}

#pragma warning(push)
#pragma warning(disable: 4290)
void* operator new(size_t size) throw (std::bad_alloc) {return DebugAllocate(size, AllocationType_Single);}
void* operator new(size_t size, const std::nothrow_t&) throw () {return DebugAllocate(size, AllocationType_Single);}
void* operator new[](size_t size) throw (std::bad_alloc) {return DebugAllocate(size, AllocationType_Array);}
void* operator new[](size_t size, const std::nothrow_t&) throw () {return DebugAllocate(size, AllocationType_Array);}
void operator delete(void* ptr) throw () {DebugDeallocate(ptr, AllocationType_Single);}
void operator delete(void* ptr, const std::nothrow_t&) throw () {DebugDeallocate(ptr, AllocationType_Single);}
void operator delete[](void* ptr) throw () {DebugDeallocate(ptr, AllocationType_Array);}
void operator delete[](void* ptr, const std::nothrow_t&) throw () {DebugDeallocate(ptr, AllocationType_Array);}
#pragma warning(pop)
