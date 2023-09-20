// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{
// Instantiates an array template when requested
BoundType* InstantiateArray(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData);

// For every instantiated array, it may want to look up information about what
// it Contains
class ArrayUserData
{
public:
  ArrayUserData();

  Type* ContainedType;
  BoundType* RangeType;
  BoundType* SelfType;
};

// This is the base definition of our array class
// It is not technically the leaf level class, but it is binary compatible with
// the Raverie Array template We add no members or virtuals in the derived class,
// and static assert that the sizes are the same
template <typename T>
class ArrayClass
{
public:
  // Constructor
  ArrayClass() : ModifyId(0)
  {
  }

  // Invalidates all active ranges via incrementing a modification id
  void Modified()
  {
    ++this->ModifyId;
  }

  // Our array is actually just an array of Any types
  // but for arrays of primitive/built in types, it will be optimized
  Array<T> NativeArray;

  // A special counter that we use to denote whenever the container has been
  // modified
  Integer ModifyId;
};

// These are all the specializations that are optimized to store exactly that
// data type All values that are unknown will be stored as the 'Any' type
#define RaverieDeclareDefineArray(ElementType)                                                                                                                                                         \
  RaverieDeclareCustomType(ArrayClass<ElementType>,                                                                                                                                                    \
                           ::Raverie::Core::GetInstance()                                                                                                                                              \
                               .GetBuilder()                                                                                                                                                           \
                               ->InstantiateTemplate("Array", RaverieConstants(RaverieTypeId(ElementType)), LibraryArray(RaverieInit, Core::GetInstance().GetBuilder()->BuiltLibrary))                 \
                               .Type);

#define RaverieDeclareDefineValueArray(ElementType) RaverieDeclareDefineArray(ElementType) typedef ArrayClass<ElementType> Array##ElementType;

#define RaverieDeclareDefineHandleArray(ElementType) RaverieDeclareDefineArray(HandleOf<ElementType>) typedef ArrayClass<HandleOf<ElementType>> Array##ElementType;

// Pre-existing useful declarations
typedef HandleOf<String> HandleOfString;
RaverieDeclareDefineValueArray(Handle);
RaverieDeclareDefineValueArray(Delegate);
RaverieDeclareDefineValueArray(Boolean);
RaverieDeclareDefineValueArray(Boolean2);
RaverieDeclareDefineValueArray(Boolean3);
RaverieDeclareDefineValueArray(Boolean4);
RaverieDeclareDefineValueArray(Byte);
RaverieDeclareDefineValueArray(Integer);
RaverieDeclareDefineValueArray(Integer2);
RaverieDeclareDefineValueArray(Integer3);
RaverieDeclareDefineValueArray(Integer4);
RaverieDeclareDefineValueArray(Real);
RaverieDeclareDefineValueArray(Real2);
RaverieDeclareDefineValueArray(Real3);
RaverieDeclareDefineValueArray(Real4);
RaverieDeclareDefineValueArray(Quaternion);
RaverieDeclareDefineValueArray(DoubleInteger);
RaverieDeclareDefineValueArray(DoubleReal);
RaverieDeclareDefineValueArray(Any);
RaverieDeclareDefineHandleArray(String);
} // namespace Raverie
