///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2014-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class GenericPhysicsMesh;

#define DefinePhysicsRuntimeClone(ResourceType)                                               \
  HandleOf<Resource> ResourceType::Clone()                                                    \
  {                                                                                           \
    return RuntimeClone();                                                                    \
  }                                                                                           \
                                                                                              \
  HandleOf<ResourceType> ResourceType::RuntimeClone()                                         \
  {                                                                                           \
    AutoDeclare(manager, GetManager());                                                       \
    HandleOf<ResourceType> cloneHandle = manager->CreateRuntimeInternal();                    \
    ResourceType* clone = cloneHandle;                                                        \
    CopyTo(clone);                                                                            \
    return cloneHandle;                                                                       \
  }

template <typename ArrayType>
class BoundMeshDataRange;

/// Base functionality for binding a mesh array (on a resource) in physics. Currently designed as
/// a templated base to reduce code duplication, especially while prototyping. Possibly split to
/// individual classes (due to differences, code documentation, etc...) or make generic enough to use elsewhere later.
template <typename OwningType, typename DataType>
class BoundMeshData : public SafeId32Object
{
public:
  typedef BoundMeshData<OwningType, DataType> SelfType;
  typedef DataType DataType;
  typedef Array<DataType> ArrayType;
  typedef BoundMeshDataRange<SelfType> RangeType;

  BoundMeshData()
  {
    mOwner = nullptr;
    mBoundArray = nullptr;
  }

  BoundMeshData(GenericPhysicsMesh* owner, ArrayType* arrayData)
  {
    mOwner = owner;
    mBoundArray = arrayData;
  }

  void Add(const DataType& vertex)
  {
    (*mBoundArray).PushBack(vertex);
    mOwner->ResourceModified();
  }

  void RemoveAt(uint arrayIndex)
  {
    if(!ValidateArrayIndex(arrayIndex))
      return;

    (*mBoundArray).EraseAt(arrayIndex);
    mOwner->ResourceModified();
  }

  Vec3Param operator[](size_t index) const
  {
    return (*mBoundArray)[index];
  }

  DataType Get(int arrayIndex) const
  {
    if(!ValidateArrayIndex(arrayIndex))
      return DataType();

    return (*mBoundArray)[arrayIndex];
  }

  void Set(int arrayIndex, const DataType value)
  {
    if(!ValidateArrayIndex(arrayIndex))
      return;

    (*mBoundArray)[arrayIndex] = value;
    mOwner->ResourceModified();
  }

  void Clear()
  {
    (*mBoundArray).Clear();
    mOwner->ResourceModified();
  }

  int GetCount() const
  {
    return (int)(*mBoundArray).Size();
  }

  RangeType GetAll()
  {
    return RangeType(this);
  }

  bool ValidateArrayIndex(int arrayIndex) const
  {
    int count = GetCount();
    if(arrayIndex >= count)
    {
      String msg = String::Format("Index %d is invalid. Array only contains %d element(s).", arrayIndex, count);
      DoNotifyException("Invalid index", msg);
      return false;
    }
    return true;
  }

  /// The array that this class represents. Assumes that this data cannot go away without this class also going away.
  ArrayType* mBoundArray;
  /// The owner of the bound array. This object is notified when the resource
  /// is modified via the calling of the "ResourceModified" function.
  OwningType* mOwner;
};

template <typename ArrayType>
class BoundMeshDataRange
{
public:
  typedef BoundMeshDataRange<ArrayType> SelfType;
  typedef ArrayType ArrayType;
  // Required for binding
  typedef typename ArrayType::DataType FrontResult;

  BoundMeshDataRange()
  {
    mArray = nullptr;
    mIndex = 0;
  }

  BoundMeshDataRange(ArrayType* array)
  {
    mArray = array;
    mIndex = 0;
  }

  bool Empty()
  {
    // Validate that the range hasn't been destroyed
    ArrayType* array = mArray;
    if(array == nullptr)
    {
      DoNotifyException("Range is invalid", "The array this range is referencing has been destroyed.");
      return true;
    }

    return mIndex >= (size_t)array->GetCount();
  }

  FrontResult Front()
  {
    // If the range is empty (or the range has been destroyed) then throw an exception.
    if(Empty())
    {
      DoNotifyException("Invalid Range Operation", "Cannot access an item in an empty range.");
      return FrontResult();
    }
    return mArray->Get(mIndex);
  }

  void PopFront()
  {
    ++mIndex;
  }

  SelfType& All()
  {
    return *this;
  }

private:
  HandleOf<ArrayType> mArray;
  size_t mIndex;
};

//-------------------------------------------------------------------PhysicsMeshVertexData
class PhysicsMeshVertexData : public BoundMeshData<GenericPhysicsMesh, Vec3>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
};

//-------------------------------------------------------------------PhysicsMeshIndexData
class PhysicsMeshIndexData : public BoundMeshData<GenericPhysicsMesh, uint>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
};

#define PhysicsDefineArrayType(arrayType)   \
  ZilchDefineType(arrayType, builder, type) \
  {                                         \
    ZeroBindDocumented();                   \
                                            \
    ZilchBindMethod(Get);                   \
    ZilchBindMethod(Set);                   \
    ZilchBindMethod(Add);                   \
    ZilchBindMethod(Clear);                 \
    ZilchBindGetter(All);                   \
    ZilchBindGetterProperty(Count);         \
  }

}//namespace Zero
