////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//--------------------------------------------------------------------------------------- Meta Array
///Meta Interface for dynamic containment of objects / components.
class MetaArray : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  MetaArray(BoundType* containedType);

  /// Returns the size of the container.
  virtual uint Size(HandleParam container) = 0;

  /// Returns the value at the given index.
  virtual Any GetValue(HandleParam container, uint index) = 0;

  /// Returns the index of the given value.
  virtual uint FindIndex(HandleParam container, AnyParam value);

  /// Gets the value with the given unique id (child id). This allows multiple
  /// Components of the same type to be referenced.
  virtual Any GetValueWithUniqueId(HandleParam container, u64 uniqueId);

  /// Adds the given value at the given index.
  virtual void Add(HandleParam container, AnyParam value) = 0;

  /// Removal of data.
  virtual bool EraseValue(HandleParam container, AnyParam value) = 0;
  virtual void EraseIndex(HandleParam container, uint index) = 0;

  struct Range
  {
    bool Empty();
    Any Front();
    void PopFront();

    Handle mContainer;
    MetaArray* mMetaArray;
    uint mIndex;
  };

  /// Range of all values in the container.
  Range All(HandleParam container);

  BoundType* mContainedType;
};

//------------------------------------------------------------------------------- Meta Array Wrapper
///Meta Interface for dynamic containment of objects / components.
class MetaArrayWrapper : public MetaArray
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MetaArrayWrapper(BoundType* typeToWrap) : 
    MetaArray(nullptr)
  {
    mContainedMetaArray = typeToWrap->HasInherited<MetaArray>();
    mContainedType = mContainedMetaArray->mContainedType;
  }

  uint Size(HandleParam container) override
  {
    return mContainedMetaArray->Size(container);
  }

  Any GetValue(HandleParam container, uint index) override
  {
    return mContainedMetaArray->GetValue(container, index);
  }

  uint FindIndex(HandleParam container, AnyParam value) override
  {
    return mContainedMetaArray->FindIndex(container, value);
  }

  Any GetValueWithUniqueId(HandleParam container, u64 uniqueId) override
  {
    return mContainedMetaArray->GetValueWithUniqueId(container, uniqueId);
  }

  void Add(HandleParam container, AnyParam value) override
  {
    mContainedMetaArray->Add(container, value);
  }

  bool EraseValue(HandleParam container, AnyParam value) override
  {
    return mContainedMetaArray->EraseValue(container, value);
  }

  void EraseIndex(HandleParam container, uint index) override
  {
    mContainedMetaArray->EraseIndex(container, index);
  }

  // We want to cache this so we aren't constantly looking it up.
  MetaArray* mContainedMetaArray;
};

}//namespace Zero
