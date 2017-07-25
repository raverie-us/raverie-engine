///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------------------- Bound Type Handle
//**************************************************************************************************
BoundTypeHandle::BoundTypeHandle(BoundType* boundType)
{
  if(boundType)
    mName = boundType->Name;
}

//**************************************************************************************************
BoundTypeHandle::operator BoundType*() const
{
  if(mName.Empty())
    return nullptr;
  return MetaDatabase::GetInstance()->FindType(mName);
}

//**************************************************************************************************
BoundType* BoundTypeHandle::operator->() const
{
  if(mName.Empty())
    return nullptr;
  return MetaDatabase::GetInstance()->FindType(mName);
}

//---------------------------------------------------------------------------------- Property Handle
//**************************************************************************************************
PropertyHandle::PropertyHandle(Property* property)
{
  if(property)
  {
    mOwner = Type::GetBoundType(property->Owner);
    mName = property->Name;
  }
}

//**************************************************************************************************
PropertyHandle::operator Property*() const
{
  return GetProperty();
}

//**************************************************************************************************
Property* PropertyHandle::operator->() const
{
  return GetProperty();
}

//**************************************************************************************************
Property* PropertyHandle::GetProperty() const
{
  // We don't want to search the base types because the owning type is the exact type that had 
  // this property. If we searched base types, the property could have been moved from our type
  // to a base type, and it's now a different property. This Handle should not return that new
  // property.
  if(BoundType* owner = mOwner)
    return owner->FindProperty(mName, FindMemberOptions::DoNotIncludeBaseClasses);
  return nullptr;
}

}//namespace Zero
