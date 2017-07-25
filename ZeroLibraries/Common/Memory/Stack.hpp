///////////////////////////////////////////////////////////////////////////////
///
/// \file Stack.hpp
/// Declaration of the Stack memory allocator.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Containers/Array.hpp"
#include "Graph.hpp"

namespace Zero
{
namespace Memory
{

static const bool StackDebug = true;

///The stack allocator works like the program stack. For every allocation the stack
///head is moved forward and for each deallocation it is moved back. 
///It is extremely efficient but allocations must be freed in stack order (LIFO). 
///This makes the stack allocator efficient for temporaries and scratch space 
///that exist only a part of a frame or for permanent allocations that are never
///freed.
class Stack : public Graph
{
public:
  //Entries are used to detect when allocation are not freed
  //in proper stack order.
  struct Entry
  {
    Entry()
    {

    }
    Entry(byte* ptr, size_t size)
      :Ptr(ptr),
      Size(size)
    {

    }

    byte* Ptr;
    size_t Size;
  };

  Stack(cstr name, Graph* parent, size_t stackSize, size_t maxEntries);
  ~Stack();
  void Print(size_t tabs, size_t flags);
  MemPtr Allocate(size_t numberOfBytes);
  void Deallocate(MemPtr ptr, size_t numberOfBytes);
  void CleanUp();
private:
  size_t mStackIndex;
  byte* mStackHeader;
  size_t mStackSize;
  size_t mMaxEntries;
  size_t mMaxSizeReached;
  PodArray<Entry> mEntries;
};

}//namespace Memory

class StackAllocator : public Memory::StandardMemory
{
public:
  StackAllocator()
    :mStack(nullptr)
  {
  }

  StackAllocator(Memory::Stack* manager)
    :mStack(manager)
  {
  }

  MemPtr Allocate(size_t numberOfBytes){return mStack->Allocate(numberOfBytes); };
  void Deallocate(MemPtr ptr, size_t numberOfBytes){mStack->Deallocate(ptr, numberOfBytes);}
  Memory::Stack* mStack;
};

}//namespace Zero
