// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class GenericPhysicsMesh;

#define DefinePhysicsRuntimeClone(ResourceType)                                                                                                                                                        \
  HandleOf<Resource> ResourceType::Clone()                                                                                                                                                             \
  {                                                                                                                                                                                                    \
    return RuntimeClone();                                                                                                                                                                             \
  }                                                                                                                                                                                                    \
                                                                                                                                                                                                       \
  HandleOf<ResourceType> ResourceType::RuntimeClone()                                                                                                                                                  \
  {                                                                                                                                                                                                    \
    AutoDeclare(manager, GetManager());                                                                                                                                                                \
    HandleOf<ResourceType> cloneHandle(manager->CreateRuntimeInternal());                                                                                                                              \
    ResourceType* clone = *cloneHandle;                                                                                                                                                                \
    CopyTo(clone);                                                                                                                                                                                     \
    return cloneHandle;                                                                                                                                                                                \
  }

template <typename ArrayType>
class BoundMeshDataRange;

/// Base functionality for binding a mesh array (on a resource) in physics.
/// Currently designed as a templated base to reduce code duplication,
/// especially while prototyping. Possibly split to individual classes (due to
/// differences, code documentation, etc...) or make generic enough to use
/// elsewhere later.
template <typename OwningType, typename DataTypeT>
class BoundMeshData : public SafeId32Object
{
public:
  typedef BoundMeshData<OwningType, DataTypeT> SelfType;
  typedef DataTypeT DataType;
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

  void Add(const DataTypeT& vertex)
  {
    (*mBoundArray).PushBack(vertex);
    mOwner->ResourceModified();
  }

  void RemoveAt(uint arrayIndex)
  {
    if (!ValidateArrayIndex(arrayIndex))
      return;

    (*mBoundArray).EraseAt(arrayIndex);
    mOwner->ResourceModified();
  }

  Vec3Param operator[](size_t index) const
  {
    return (*mBoundArray)[index];
  }

  DataTypeT Get(int arrayIndex) const
  {
    if (!ValidateArrayIndex(arrayIndex))
      return DataTypeT();

    return (*mBoundArray)[arrayIndex];
  }

  void Set(int arrayIndex, const DataTypeT value)
  {
    if (!ValidateArrayIndex(arrayIndex))
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
    if (arrayIndex >= count)
    {
      String msg = String::Format("Index %d is invalid. Array only contains %d element(s).", arrayIndex, count);
      DoNotifyException("Invalid index", msg);
      return false;
    }
    return true;
  }

  /// The array that this class represents. Assumes that this data cannot go
  /// away without this class also going away.
  ArrayType* mBoundArray;
  /// The owner of the bound array. This object is notified when the resource
  /// is modified via the calling of the "ResourceModified" function.
  OwningType* mOwner;
};

template <typename ArrayTypeT>
class BoundMeshDataRange
{
public:
  typedef BoundMeshDataRange<ArrayTypeT> SelfType;
  typedef ArrayTypeT ArrayType;
  // Required for binding
  typedef typename ArrayType::DataType FrontResult;

  BoundMeshDataRange()
  {
    mArray = nullptr;
    mIndex = 0;
  }

  BoundMeshDataRange(ArrayTypeT* array)
  {
    mArray = array;
    mIndex = 0;
  }

  bool Empty()
  {
    // Validate that the range hasn't been destroyed
    ArrayType* array = mArray;
    if (array == nullptr)
    {
      DoNotifyException("Range is invalid", "The array this range is referencing has been destroyed.");
      return true;
    }

    return mIndex >= (size_t)array->GetCount();
  }

  FrontResult Front()
  {
    // If the range is empty (or the range has been destroyed) then throw an
    // exception.
    if (Empty())
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
  HandleOf<ArrayTypeT> mArray;
  size_t mIndex;
};

class PhysicsMeshVertexData : public BoundMeshData<GenericPhysicsMesh, Vec3>
{
public:
  RaverieDeclareType(PhysicsMeshVertexData, TypeCopyMode::ReferenceType);
};

class PhysicsMeshIndexData : public BoundMeshData<GenericPhysicsMesh, uint>
{
public:
  RaverieDeclareType(PhysicsMeshIndexData, TypeCopyMode::ReferenceType);
};

#define PhysicsDefineArrayType(arrayType)                                                                                                                                                              \
  RaverieDefineType(arrayType, builder, type)                                                                                                                                                          \
  {                                                                                                                                                                                                    \
    RaverieBindDocumented();                                                                                                                                                                           \
                                                                                                                                                                                                       \
    RaverieBindMethod(Get);                                                                                                                                                                            \
    RaverieBindMethod(Set);                                                                                                                                                                            \
    RaverieBindMethod(Add);                                                                                                                                                                            \
    RaverieBindMethod(Clear);                                                                                                                                                                          \
    RaverieBindGetter(All);                                                                                                                                                                            \
    RaverieBindGetterProperty(Count);                                                                                                                                                                  \
  }

} // namespace Raverie
