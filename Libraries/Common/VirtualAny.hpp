////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Nathan Carlson
/// Copyright 2016, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

template <typename BaseType, uint MaxDerivedSize>
class VirtualAny
{
public:

  VirtualAny()
  {
    mBasePointer = nullptr;
    mCopyConstructor = nullptr;
  }

  ~VirtualAny()
  {
    VirtualDestructor();
  }

  VirtualAny(const VirtualAny& other)
  {
    mBasePointer = nullptr;
    mCopyConstructor = nullptr;
    Copy(other);
  }

  VirtualAny& operator=(const VirtualAny& other)
  {
    VirtualDestructor();
    Copy(other);
    return *this;
  }

  template <typename DerivedType>
  VirtualAny(const DerivedType& other)
  {
    mBasePointer = (BaseType*)(new(mObjectData) DerivedType(other));
    mCopyConstructor = &CopyConstructor<DerivedType>;
  }

  BaseType& operator*()
  {
    return *mBasePointer;
  }

  BaseType* operator->()
  {
    return mBasePointer;
  }

protected:

  void VirtualDestructor()
  {
    if (mBasePointer != nullptr)
      mBasePointer->~BaseType();
    mBasePointer = nullptr;
    mCopyConstructor = nullptr;
  }

  void Copy(const VirtualAny& other)
  {
    if (other.mBasePointer != nullptr)
    {
      mBasePointer = other.mCopyConstructor(mObjectData, other.mBasePointer);
      mCopyConstructor = other.mCopyConstructor;
    }
  }

  typedef BaseType* (*CopyConstructorFn)(byte*, const BaseType*);

  template <typename T>
  static BaseType* CopyConstructor(byte* dest, const BaseType* source)
  {
    return (BaseType*)(new(dest) T(*(const T*)source));
  }

  BaseType* mBasePointer;
  CopyConstructorFn mCopyConstructor;
  byte mObjectData[MaxDerivedSize];

  friend struct MoveWithoutDestructionOperator< VirtualAny<BaseType, MaxDerivedSize> >;
};

template <typename BaseType, uint MaxDerivedSize>
struct MoveWithoutDestructionOperator< VirtualAny<BaseType, MaxDerivedSize> >
{
  typedef VirtualAny<BaseType, MaxDerivedSize> VirtualAnyType;

  static inline void MoveWithoutDestruction(VirtualAnyType* dest, VirtualAnyType* source)
  {
    memcpy(dest->mObjectData, source->mObjectData, sizeof(source->mObjectData));
    dest->mBasePointer = (BaseType*)dest->mObjectData;
    dest->mCopyConstructor = source->mCopyConstructor;
  }
};

} // namespace Zero
