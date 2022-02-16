// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
namespace Memory
{

Heap::Heap(cstr name, Graph* parent) : Graph(name, parent)
{
  //
}

MemPtr Heap::Allocate(size_t numberOfBytes)
{
  AddAllocation(numberOfBytes);
  MemPtr mem = zAllocate(numberOfBytes);
  return mem;
}

void Heap::Deallocate(MemPtr ptr, size_t numberOfBytes)
{
  RemoveAllocation(numberOfBytes);
  zDeallocate(ptr);
}

void Heap::Print(size_t tabs, size_t flags)
{
  PrintHelper(tabs, flags, "Heap");
}

} // namespace Memory
} // namespace Zero
