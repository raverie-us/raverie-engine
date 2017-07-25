/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  ZilchThreadLocal bool BuildingLibrary = false;

  //***************************************************************************
  NativeBindingList::NativeBindingList()
  {
  }

  //***************************************************************************
  NativeBindingList& NativeBindingList::GetInstance()
  {
    static NativeBindingList instance;
    return instance;
  }

  //***************************************************************************
  bool NativeBindingList::IsBuildingLibrary()
  {
    return BuildingLibrary;
  }

  //***************************************************************************
  void NativeBindingList::SetBuildingLibraryForThisThread(bool value)
  {
    BuildingLibrary = value;
  }

  //***************************************************************************
  void NativeBindingList::ValidateTypes()
  {
    NativeBindingList& self = GetInstance();
    self.Lock.Lock();

    ZilchForEach(BoundType* type, self.AllNativeBoundTypes)
    {
      type->IsInitializedAssert();
    }

    self.Lock.Unlock();
  }

  //***************************************************************************
  BoundType* NoType::ZilchGetDerivedType() const
  {
    return nullptr;
  }

  //***************************************************************************
  bool TypeBinding::VirtualTableCounter::StaticDebugIsVirtual = false;
  
  //***************************************************************************
  TypeBinding::VirtualTableCounter::VirtualTableCounter()
  {
    // Make sure both flags are set to false so that we don't trip the assert
    StaticDebugIsVirtual = false;
    this->InstanceDebugIsVirtual = false;
  }
  
  //***************************************************************************
  void TypeBinding::VirtualTableCounter::AssertIfNotVirtual()
  {
    // Perform the error checking
    ErrorIf(StaticDebugIsVirtual == false || this->InstanceDebugIsVirtual == false,
      "Method being tested was not virtual!");

    // Reset our state back, just in case we use this again
    StaticDebugIsVirtual = false;
    this->InstanceDebugIsVirtual = false;
  }

  //***************************************************************************
  BoundType* TypeBinding::StaticTypeId<void>::GetType()
  {
    return Core::GetInstance().VoidType;
  }

  //***************************************************************************
  BoundType* TypeBinding::StaticTypeId<NoType>::GetType()
  {
    return nullptr;
  }

  //***************************************************************************
  BoundType* TypeBinding::StaticTypeId<NullPointerType>::GetType()
  {
    return Core::GetInstance().NullType;
  }

  //***************************************************************************
  AnyType* TypeBinding::StaticTypeId<Any>::GetType()
  {
    return Core::GetInstance().AnythingType;
  }

  //***************************************************************************
  DelegateType* TypeBinding::StaticTypeId<Delegate>::GetType()
  {
    return Core::GetInstance().AnyDelegateType;
  }

  //***************************************************************************
  BoundType* TypeBinding::StaticTypeId<Handle>::GetType()
  {
    return Core::GetInstance().AnyHandleType;
  }

  //***************************************************************************
  AutoGrabAllocatingType::AutoGrabAllocatingType()
  {
    if (ExecutableState::CallingState != nullptr)
      this->Type = ExecutableState::CallingState->AllocatingType;
    else
      this->Type = nullptr;
  }

  //***************************************************************************
  ZilchDefineExternalBaseType(Boolean,        TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(Boolean2,       TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(Boolean3,       TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(Boolean4,       TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(Byte,           TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(Integer,        TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(Integer2,       TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(Integer3,       TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(Integer4,       TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(Real,           TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(Real2,          TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(Real3,          TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(Real4,          TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(Quaternion,     TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(Real2x2,        TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(Real3x3,        TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(Real4x4,        TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(String,         TypeCopyMode::ReferenceType,  builder, type) {}
  ZilchDefineExternalBaseType(DoubleReal,     TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(DoubleInteger,  TypeCopyMode::ValueType,      builder, type) {}
  ZilchDefineExternalBaseType(Zero::Rune,     TypeCopyMode::ValueType, builder, type) {}
}
