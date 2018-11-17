///////////////////////////////////////////////////////////////////////////////
///
/// \file Block.cpp
/// Implementation of the block memory manger and allocator.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
namespace Memory
{

byte Block::BucketLookUp[cMaxBlockSize+1];
bool Block::SizeTableInitialized = false;

size_t Block::BlockSizes[cBlockCount] = 
{
  16,//[0]
  32,//[1]
  64,//[2]
  96,//[3]
  128,//[4]
  160,//[5]
  192,//[6]
  224,//[7]
  256,//[8]
  320,//[9]
  384,//[10]
  448,//[11]
  512,//[12]
  640,//[13]
};

Block::Block(cstr name, Graph* parent)
  :Graph(name, parent)
{
  for(size_t i=0;i<cBlockCount;++i)
    mBlockArray[i] = nullptr;

  if(!SizeTableInitialized)
  {
    size_t currentBlock = 0;
    for(size_t i=0;i<cMaxBlockSize+1;++i)
    {
      if(i>BlockSizes[currentBlock])
        ++currentBlock;

      BucketLookUp[i] = (byte)currentBlock;
    }

    SizeTableInitialized = true;
  }
}

Block::~Block()
{
  CleanUp();
}

MemPtr Block::Allocate(size_t numberOfBytes)
{
  AddAllocation(numberOfBytes);

  //Determine what bucket to use
  ErrorIf(numberOfBytes > cMaxBlockSize, "Size is larger than max block size. "
    "This allocator can only allocate small objects");

  size_t bucketIndex = BucketLookUp[numberOfBytes];

  return PopOnFreeList(bucketIndex);
}

Block::FreeBlock* Block::PopOnFreeList(size_t blockIndex)
{
  if(mBlockArray[blockIndex] == nullptr)
    AllocateBlockPage(blockIndex);

  FreeBlock* block = mBlockArray[blockIndex];
  mBlockArray[blockIndex] = block->NextBlock;

  return block;
}

void Block::Deallocate(MemPtr ptr, size_t numberOfBytes)
{
  RemoveAllocation(numberOfBytes);

  //Determine what bucket to use
  ErrorIf(numberOfBytes > cMaxBlockSize, "Size is larger than max block size. "
    "This allocator can only allocate small objects");

  size_t bucketIndex = BucketLookUp[numberOfBytes];
  PushFreeBlock(bucketIndex, (FreeBlock*)ptr);
}

void Block::PushFreeBlock(size_t blockIndex, FreeBlock* block)
{
  block->NextBlock = mBlockArray[blockIndex];
  mBlockArray[blockIndex] = block;
}

void Block::AllocateBlockPage(size_t blockIndex)
{
  size_t blockSize = BlockSizes[blockIndex];
  size_t blocksOnPage = cPageSize / blockSize;

  byte* memoryPage = (byte*)zAllocate(cPageSize);
  DeltaDedicated(cPageSize);

  for(size_t i=0;i<blocksOnPage;++i)
    PushFreeBlock(blockIndex, (FreeBlock*)(memoryPage+blockSize*i));

  mPageBlocks.PushBack(memoryPage);
}

void Block::Print(size_t tabs, size_t flags)
{
  PrintHelper(tabs, flags, "Block");
}

void Block::CleanUp()
{
  Array<MemPtr>::range blocksToFree = mPageBlocks.All();
  while(!blocksToFree.Empty())
  {
    zDeallocate(blocksToFree.Front());
    blocksToFree.PopFront();
  }
  mPageBlocks.Clear();
}

}//namespace Memory
}//namespace Zero
