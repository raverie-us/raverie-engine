// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{
// @TrevorS: Move this to be filled out automatically in my binding!
BoundType* ObjectBindingVirtualTypeFn(const byte* memory)
{
  const Object* object = (const Object*)memory;
  return object->RaverieGetDerivedType();
}

RaverieDefineType(Object, builder, type)
{
  type->GetBindingVirtualType = &ObjectBindingVirtualTypeFn;
}

Memory::Heap* sGeneralPool = NULL;

void* Object::operator new(size_t size)
{
  if (sGeneralPool == NULL)
    sGeneralPool = new Memory::Heap("System", Memory::GetRoot());
  return sGeneralPool->Allocate(size);
};
void Object::operator delete(void* pMem, size_t size)
{
  sGeneralPool->Deallocate(pMem, size);
}

Object::~Object()
{
}

bool Object::SetProperty(StringParam propertyName, AnyParam val)
{
  Property* prop = RaverieVirtualTypeId(this)->GetProperty(propertyName);
  if (prop == nullptr)
    return false;
  prop->SetValue(this, val);
  return true;
}

Any Object::GetProperty(StringParam propertyName)
{
  Property* prop = RaverieVirtualTypeId(this)->GetProperty(propertyName);
  if (prop)
    return prop->GetValue(this);
  return Any();
}

} // namespace Raverie
