////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//--------------------------------------------------------------------------------------- Meta Array
ZilchDefineType(MetaArray, builder, type)
{
}

ZilchDefineType(MetaArrayWrapper, builder, type)
{
}

//**************************************************************************************************
MetaArray::MetaArray(BoundType* containedType) : 
  mContainedType(containedType)
{

}

//**************************************************************************************************
uint MetaArray::FindIndex(HandleParam container, AnyParam value)
{
  uint size = Size(container);
  for(uint i = 0; i < size; ++i)
  {
    if(GetValue(container, i) == value)
      return i;
  }
  return uint(-1);
}

//**************************************************************************************************
Any MetaArray::GetValueWithUniqueId(HandleParam instance, u64 uniqueId)
{
  Error("Not implemented");
  return Any();
}

//**************************************************************************************************
bool MetaArray::Range::Empty()
{
  return (mIndex >= mMetaArray->Size(mContainer));
}

//**************************************************************************************************
Any MetaArray::Range::Front()
{
  return mMetaArray->GetValue(mContainer, mIndex);
}

//**************************************************************************************************
void MetaArray::Range::PopFront()
{
  ++mIndex;
}

//**************************************************************************************************
MetaArray::Range MetaArray::All(HandleParam container)
{
  Range r;
  r.mContainer = container;
  r.mIndex = 0;
  r.mMetaArray = this;
  return r;
}

}//namespace Zero
