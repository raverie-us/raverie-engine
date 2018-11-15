///////////////////////////////////////////////////////////////////////////////
///
/// \file Allocator.hpp
/// Declaration of the  Allocator interface and the Default Allocator.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef UseMemoryGraph
#define UseMemoryGraph 1
#endif

#if UseMemoryGraph

#include "Memory/Graph.hpp"
#include "Memory/Heap.hpp"

#else

namespace Zero
{

void* zAllocate(size_t numberOfBytes);
void zDeallocate(void*);

class ZeroShared StandardMemory
{
public:
  static inline void MemCopy(void* dest, void* source, size_t numberOfBytes)
  {
    memcpy(dest, source, numberOfBytes);
  }

  static inline void MemMove(void* dest, void* source, size_t numberOfBytes)
  {
    memmove(dest, source, numberOfBytes);
  }
};

//Default allocator of Standard Memory. This allocator
//is used by default for all the containers.
class ZeroShared DefaultAllocator : public StandardMemory
{
public:
  enum{ cAlignment = 4 };
  void* Allocate(size_t numberOfBytes) { return zAllocate(numberOfBytes); };
  void Deallocate(void* ptr, size_t numberOfBytes) { zDeallocate(ptr); }
};

}//namespace Zero

#endif // UseMemoryGraph
