///////////////////////////////////////////////////////////////////////////////
///
/// \file Type.cpp
/// Implementation of the Type system.
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(Object, builder, type)
{
}

Memory::Heap* sGeneralPool = NULL;

void* Object::operator new(size_t size)
{
  if(sGeneralPool == NULL)
    sGeneralPool = new Memory::Heap("System", Memory::GetRoot());
  return sGeneralPool->Allocate(size); 
};
void Object::operator delete(void* pMem, size_t size) { sGeneralPool->Deallocate(pMem, size); }

Object::~Object()
{
}

bool Object::SetProperty(StringParam propertyName, AnyParam val)
{
  Property* prop = ZilchVirtualTypeId(this)->GetProperty(propertyName);
  if(prop == nullptr)
    return false;
  prop->SetValue(this, val);
  return true;
}

Any Object::GetProperty(StringParam propertyName)
{
  Property* prop = ZilchVirtualTypeId(this)->GetProperty(propertyName);
  if(prop)
    return prop->GetValue(this);
  return Any();
}

}//namespace Zero
