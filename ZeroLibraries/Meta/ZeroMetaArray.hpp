///////////////////////////////////////////////////////////////////////////////
///
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//---------------------------------------------------------------------------------- Zero Meta Array
template<typename ArrayType>
class ZeroMetaArray : public MetaArray
{
public:
  // We don't initialize a type for MetaContainer because we never
  // explicitly look for MetaContainer, but we do Has<MetaComposition>...
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  typedef typename ArrayType::value_type ContainedType;

  ZeroMetaArray() : MetaArray(ZilchTypeId(ContainedType))
  {
  }

  uint Size(HandleParam instance) override
  {
    ArrayType& container = *instance.Get<ArrayType*>(GetOptions::AssertOnNull);
    return container.Size();
  }

  Any GetValue(HandleParam instance, uint index) override
  {
    ArrayType& container = *instance.Get<ArrayType*>(GetOptions::AssertOnNull);
    return container[index];
  }

  void Add(HandleParam instance, AnyParam value) override
  {
    ArrayType& container = *instance.Get<ArrayType*>(GetOptions::AssertOnNull);
    ContainedType* element = value.Get<ContainedType*>(GetOptions::AssertOnNull);
    container.PushBack();
    container.Back().SetDefaults();
  }

  bool EraseValue(HandleParam instance, AnyParam value) override
  {
    //ArrayType& container = *instance.Get<ArrayType*>(GetOptions::AssertOnNull);
    //ContainedType* element = value.Get<ContainedType*>(GetOptions::AssertOnNull);
    //return container.EraseValue(*element);
    return false;
  }

  void EraseIndex(HandleParam instance, uint index) override
  {
    ArrayType& container = *instance.Get<ArrayType*>(GetOptions::AssertOnNull);
    container.EraseAt(index);
  }
};

}//namespace Zero
