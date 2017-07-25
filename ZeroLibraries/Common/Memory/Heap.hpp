///////////////////////////////////////////////////////////////////////////////
///
/// \file Heap.hpp
/// Declaration of the Heap Allocator.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Graph.hpp"

namespace Zero
{
namespace Memory
{

class HeapPrivate;

///Heap allocator. The heap allocator allocates memory directly from the 
///system heap using malloc and free.
class ZeroShared Heap : public Graph
{
public:
  Heap(cstr name, Graph* parent);
  MemPtr Allocate(size_t numberOfBytes);
  void Deallocate(MemPtr ptr, size_t numberOfBytes);

  virtual void Print(size_t tabs, size_t flags);
};

template <typename type>
type* HeapAllocate(Heap* heap)
{
  MemPtr memory = heap->Allocate(sizeof(type));
  type* object = new(memory) type();
  return object;
}

template <typename type, typename ConstructionType>
type* HeapAllocate(Heap* heap, const ConstructionType& constructionData)
{
  MemPtr memory = heap->Allocate(sizeof(type));
  type* object = new(memory) type(constructionData);
  return object;
}

template <typename type>
void HeapDeallocate(Heap* heap, type* instance)
{
  instance->~type();
  heap->Deallocate(instance,sizeof(type));
}


}//namespace Memory


#define UseStaticHeap() \
  static void* operator new(size_t size){ return Memory::GetStaticHeap()->Allocate(size); } \
  static void operator delete(void* pMem, size_t size){return Memory::GetStaticHeap()->Deallocate(pMem, size);}

template<typename NodeType>
class ZeroSharedTemplate TypedAllocator : public Memory::StandardMemory
{
  typedef TypedAllocator<NodeType> this_type;

public:
  TypedAllocator()
    :mNode(Memory::GetGlobalHeap())
  {
  }

  TypedAllocator(cstr name)
    :mNode(Memory::GetNamedHeap(name))
  {
  }

  TypedAllocator(NodeType* manager)
    :mNode(manager)
  {
  }

  MemPtr Allocate(size_t numberOfBytes){return mNode->Allocate(numberOfBytes); };
  void Deallocate(MemPtr ptr, size_t numberOfBytes){mNode->Deallocate(ptr, numberOfBytes);}
  NodeType* mNode;
};

// This allocator specifically works with 
template<typename NodeType>
class ZeroSharedTemplate MemsetZeroTypedAllocator : public TypedAllocator<NodeType>
{
public:
  MemsetZeroTypedAllocator()
    :mNode(Memory::GetGlobalHeap())
  {
  }

  MemsetZeroTypedAllocator(cstr name)
    :mNode(Memory::GetNamedHeap(name))
  {
  }

  MemsetZeroTypedAllocator(NodeType* manager)
    :mNode(manager)
  {
  }

  MemPtr Allocate(size_t numberOfBytes)
  {
    if(mNode == nullptr)
      mNode = Memory::GetGlobalHeap();
    return mNode->Allocate(numberOfBytes);
  }

  void Deallocate(MemPtr ptr, size_t numberOfBytes)
  {
    if(mNode == nullptr)
      mNode = Memory::GetGlobalHeap();
    mNode->Deallocate(ptr, numberOfBytes);
  }

  NodeType* mNode;
};

typedef TypedAllocator<Memory::Heap> HeapAllocator;

//Override default
typedef TypedAllocator<Memory::Heap> DefaultAllocator;
typedef MemsetZeroTypedAllocator<Memory::Heap> MemsetZeroDefaultAllocator;

}//namespace Zero
