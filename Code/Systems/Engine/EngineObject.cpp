// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

Memory::Heap* EngineObject::sEngineHeap = nullptr;

void* EngineObject::operator new(size_t size)
{
  return sEngineHeap->Allocate(size);
}

void EngineObject::operator delete(void* pMem, size_t size)
{
  return sEngineHeap->Deallocate(pMem, size);
}

EngineObject::~EngineObject()
{
}

} // namespace Raverie
