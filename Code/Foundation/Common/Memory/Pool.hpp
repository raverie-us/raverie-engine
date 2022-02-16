// MIT Licensed (see LICENSE.md).
#pragma once
#include "Containers/Array.hpp"
#include "Graph.hpp"

namespace Zero
{
namespace Memory
{

/// A memory pool is an efficient allocator for objects of a fixed size.
/// The memory pool allocates memory in pages then divides the page into a
/// intrusively singly linked list of free blocks. Every time an allocation
/// is made the next free block is popped of the list. Every time an allocation
/// is freed the memory is push back onto the front of the list. When their are
/// no more free blocks new pages are allocated.
class Pool : public Graph
{
public:
  struct FreeBlock
  {
    FreeBlock* NextBlock;
  };
  Pool(cstr name, Graph* parent, size_t blockSize, size_t blocksPerPage, bool podStackPool = false);
  ~Pool();

  static void* operator new(size_t size)
  {
    return malloc(size);
  }
  static void operator delete(void* pMem, size_t size)
  {
    free(pMem);
  }

  template <typename type>
  type* AllocateType();
  template <typename type, typename P0>
  type* AllocateType(P0& p0);
  template <typename type, typename P0, typename P1>
  type* AllocateType(P0& p0, P1& p1);
  template <typename type, typename P0, typename P1, typename P2>
  type* AllocateType(P0& p0, P1& p1, P2& p2);
  template <typename type, typename P0, typename P1, typename P2, typename P3>
  type* AllocateType(P0& p0, P1& p1, P2& p2, P3& p3);
  template <typename type>
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

template <typename type>
type* Pool::AllocateType()
{
  MemPtr memory = Allocate(sizeof(type));
  type* object = new (memory) type();
  return object;
}

template <typename type, typename P0>
type* Pool::AllocateType(P0& p0)
{
  MemPtr memory = Allocate(sizeof(type));
  type* object = new (memory) type(p0);
  return object;
}

template <typename type, typename P0, typename P1>
type* Pool::AllocateType(P0& p0, P1& p1)
{
  MemPtr memory = Allocate(sizeof(type));
  type* object = new (memory) type(p0, p1);
  return object;
}

template <typename type, typename P0, typename P1, typename P2>
type* Pool::AllocateType(P0& p0, P1& p1, P2& p2)
{
  MemPtr memory = Allocate(sizeof(type));
  type* object = new (memory) type(p0, p1, p2);
  return object;
}

template <typename type, typename P0, typename P1, typename P2, typename P3>
type* Pool::AllocateType(P0& p0, P1& p1, P2& p2, P3& p3)
{
  MemPtr memory = Allocate(sizeof(type));
  type* object = new (memory) type(p0, p1, p2, p3);
  return object;
}

template <typename type>
void Pool::DeallocateType(type* instance)
{
  instance->~type();
  Deallocate(instance, sizeof(type));
}

} // namespace Memory
} // namespace Zero
