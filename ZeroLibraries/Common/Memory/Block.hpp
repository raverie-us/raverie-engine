///////////////////////////////////////////////////////////////////////////////
///
/// \file Block.hpp
/// Declaration of the block memory manger and allocator.
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

///Block allocator implements a segmented memory model.
///The block allocator is an array of memory pools of fixed
///sizes. Allocations use a static look up table to find the free list
///their size will fit into.  For each free list size a intrusive singly 
///linked list of free blocks is stored just like in a memory pool.
///The Block allocator is efficient at allocating small objects and helps
///prevent memory fragmentation. The disadvantages is that
///the Block allocator can not allocate large objects 
///(with out falling back on heap allocation)
///and once memory is committed to a block size
///it can not be reclaimed easily.
class Block : public Graph
{
public:
  struct FreeBlock{FreeBlock* NextBlock;};
  Block(cstr name, Graph* parent);
  ~Block();

  //Allocate an object and call its constructor.
  template<typename type>
  type* AllocateType();

  MemPtr Allocate(size_t numberOfBytes);
  void Deallocate(MemPtr ptr, size_t numberOfBytes);
  void Print(size_t tabs, size_t flags);
  void PushFreeBlock(size_t blockIndex, FreeBlock* block);
  FreeBlock* PopOnFreeList(size_t blockIndex);
  void AllocateBlockPage(size_t blockIndex);
  void CleanUp();
  //Configuration
  static const size_t cBlockCount = 14;
  static const size_t cMaxBlockSize = 640;
  static const size_t cPageSize = 4096;//4K Pages

private:

  //Used to store pages.
  Array<MemPtr> mPageBlocks;

  //Array of FreeLists for each block size.
  FreeBlock* mBlockArray[cBlockCount];

  //Table used to look up block index from allocation size.
  static byte BucketLookUp[cMaxBlockSize+1];
  //Is the allocation table initialized.
  static bool SizeTableInitialized;

  //Size of the block lists.
  static size_t BlockSizes[cBlockCount];

};

template<typename type>
type* Block::AllocateType()
{
  MemPtr memory = Allocate(sizeof(type));
  type* object = new(memory) type();
  return object;
}

}//namespace Memory
}//namespace Zero
