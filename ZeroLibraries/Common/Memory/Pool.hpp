///////////////////////////////////////////////////////////////////////////////
///
/// \file Pool.hpp
/// Declaration of the memory pool allocator.
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

///A memory pool is an efficient allocator for objects of a fixed size.
///The memory pool allocates memory in pages then divides the page into a 
///intrusively singly linked list of free blocks. Every time an allocation
///is made the next free block is popped of the list. Every time an allocation
///is freed the memory is push back onto the front of the list. When their are 
///no more free blocks new pages are allocated.
class Pool : public Graph
{
public:
  struct FreeBlock{FreeBlock* NextBlock;};
  Pool(cstr name, Graph* parent, size_t blockSize, size_t blocksPerPage, bool podStackPool = false);
  ~Pool();

  template<typename type>
  type* AllocateType();
  template<typename type>
  void DeallocateType(type* instance);
  MemPtr Allocate(size_t numberOfBytes);
  void Deallocate(MemPtr ptr, size_t numberOfBytes);
  void Print(size_t tabs, size_t flags);
  void CleanUp();
private:
  FreeBlock* mNextFreeBlock;
  size_t mBlockSize;
  size_t mBlocksPerPage;
  size_t mPageSize;
  Array<byte*> mPages;
  void PushOnFreeList(MemPtr chunk);
  MemPtr PopOnFreeList();
  void AllocatePage();

  bool mPodStackPool;
};

template<typename type>
type* Pool::AllocateType()
{
  MemPtr memory = Allocate(sizeof(type));
  type* object = new(memory) type();
  return object;
}

template<typename type>
void Pool::DeallocateType(type* instance)
{
  instance->~type();
  Deallocate(instance,sizeof(type));
}

}//namespace Memory
}//namespace Zero
