///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------------------- Bound Type Handle
class BoundTypeHandle
{
public:
  BoundTypeHandle() {}
  BoundTypeHandle(BoundType* boundType);

  operator BoundType*() const;
  BoundType* operator->() const;

  String mName;
};

//---------------------------------------------------------------------------------- Property Handle
class PropertyHandle
{
public:
  PropertyHandle() {}
  PropertyHandle(Property* property);

  operator Property*() const;
  Property* operator->() const;
  Property* GetProperty() const;

  BoundTypeHandle mOwner;
  String mName;
};

//---------------------------------------------------------------------------- Meta Component Handle
template <typename ComponentType>
class MetaComponentHandle
{
public:
  MetaComponentHandle() {}
  MetaComponentHandle(BoundType* owner);
  MetaComponentHandle(ComponentType* metaComponent);

  operator ComponentType*() const;
  ComponentType* operator->() const;

  BoundTypeHandle mOwner;
};

//**************************************************************************************************
template <typename ComponentType>
MetaComponentHandle<ComponentType>::MetaComponentHandle(BoundType* owner) :
  mOwner(owner)
{

}

//**************************************************************************************************
template <typename ComponentType>
MetaComponentHandle<ComponentType>::MetaComponentHandle(ComponentType* metaComponent)
{
  if(metaComponent)
    mOwner = metaComponent->mOwner;
}

//**************************************************************************************************
template <typename ComponentType>
MetaComponentHandle<ComponentType>::operator ComponentType*() const
{
  if(BoundType* owner = mOwner)
    return owner->HasInherited<ComponentType>();
  return nullptr;
}

//**************************************************************************************************
template <typename ComponentType>
ComponentType* MetaComponentHandle<ComponentType>::operator->() const
{
  if(BoundType* owner = mOwner)
    return owner->HasInherited<ComponentType>();
  return nullptr;
}

}//namespace Zero
