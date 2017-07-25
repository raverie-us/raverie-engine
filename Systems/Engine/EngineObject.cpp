///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

Memory::Heap* EngineObject::sEngineHeap = nullptr;

//******************************************************************************
void* EngineObject::operator new(size_t size)
{
  return sEngineHeap->Allocate(size);
}

//******************************************************************************
void EngineObject::operator delete(void* pMem, size_t size)
{
  return sEngineHeap->Deallocate(pMem, size);
}

//******************************************************************************
EngineObject::~EngineObject()
{

}

}// namespace Zero
