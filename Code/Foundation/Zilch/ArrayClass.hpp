// MIT Licensed (see LICENSE.md).

#pragma once
#ifndef ZILCH_ARRAY_HPP
#  define ZILCH_ARRAY_HPP

namespace Zilch
{
// Instantiates an array template when requested
BoundType* InstantiateArray(LibraryBuilder& builder,
                                       StringParam baseName,
                                       StringParam fullyQualifiedName,
                                       const Array<Constant>& templateTypes,
                                       const void* userData);

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
// the Zilch Array template We add no members or virtuals in the derived class,
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
#  define ZilchDeclareDefineArray(ElementType)                                                                         \
    ZilchDeclareCustomType(                                                                                            \
        ArrayClass<ElementType>,                                                                                       \
        ZZ::Core::GetInstance()                                                                                        \
            .GetBuilder()                                                                                              \
            ->InstantiateTemplate("Array",                                                                             \
                                  ZilchConstants(ZilchTypeId(ElementType)),                                            \
                                  LibraryArray(ZeroInit, Core::GetInstance().GetBuilder()->BuiltLibrary))              \
            .Type);

#  define ZilchDeclareDefineValueArray(ElementType)                                                           \
    ZilchDeclareDefineArray(ElementType) typedef ArrayClass<ElementType> Array##ElementType;

#  define ZilchDeclareDefineHandleArray(ElementType)                                                          \
    ZilchDeclareDefineArray(HandleOf<ElementType>) typedef ArrayClass<HandleOf<ElementType>>                  \
        Array##ElementType;

// Pre-existing useful declarations
typedef HandleOf<String> HandleOfString;
ZilchDeclareDefineValueArray(Handle);
ZilchDeclareDefineValueArray(Delegate);
ZilchDeclareDefineValueArray(Boolean);
ZilchDeclareDefineValueArray(Boolean2);
ZilchDeclareDefineValueArray(Boolean3);
ZilchDeclareDefineValueArray(Boolean4);
ZilchDeclareDefineValueArray(Byte);
ZilchDeclareDefineValueArray(Integer);
ZilchDeclareDefineValueArray(Integer2);
ZilchDeclareDefineValueArray(Integer3);
ZilchDeclareDefineValueArray(Integer4);
ZilchDeclareDefineValueArray(Real);
ZilchDeclareDefineValueArray(Real2);
ZilchDeclareDefineValueArray(Real3);
ZilchDeclareDefineValueArray(Real4);
ZilchDeclareDefineValueArray(Quaternion);
ZilchDeclareDefineValueArray(DoubleInteger);
ZilchDeclareDefineValueArray(DoubleReal);
ZilchDeclareDefineValueArray(Any);
ZilchDeclareDefineHandleArray(String);
} // namespace Zilch

#endif
