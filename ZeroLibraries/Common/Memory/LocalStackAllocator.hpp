///////////////////////////////////////////////////////////////////////////////
///
/// \file LocalStackAllocator.hpp
/// Declaration of the Local Stack Allocator.
///
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Memory/Graph.hpp"

namespace Zero
{

///Allocator to be used with stack memory (memory from alloca). The problem
///with stack memory is that it gets freed when the current scope exits, so
///this allocator cannot actually allocate the memory. The memory must be
///handed to the allocator to use when requested. And since this allocator
///may be used with arrays, it is only allowed to allocate once
///(a reserve operation) since it cannot grow itself.
class LocalStackAllocator : public Memory::StandardMemory
{
public:
  LocalStackAllocator()
    : mMemory(nullptr), mHasAllocated(false)
  {

  }

  LocalStackAllocator(void* memory)
    :mMemory(memory), mHasAllocated(false)
  {

  }

  MemPtr Allocate(size_t numberOfBytes)
  {
    ErrorIf(mHasAllocated,"Cannot allocate more memory with a local stack "
      "allocator. Please reserve all memory that is needed up front.");
    mHasAllocated = true;

    return mMemory;
  }

  void Deallocate(MemPtr ptr, size_t numberOfBytes)
  {
    mHasAllocated = false;
  }

  void* mMemory;
  bool mHasAllocated;
};

}//namespace Zero
