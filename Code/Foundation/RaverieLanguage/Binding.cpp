// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{
RaverieThreadLocal bool BuildingLibrary = false;

NativeBindingList::NativeBindingList()
{
}

NativeBindingList& NativeBindingList::GetInstance()
{
  static NativeBindingList instance;
  return instance;
}

bool NativeBindingList::IsBuildingLibrary()
{
  return BuildingLibrary;
}

void NativeBindingList::SetBuildingLibraryForThisThread(bool value)
{
  BuildingLibrary = value;
}

void NativeBindingList::ValidateTypes()
{
#if RaverieDebug
  NativeBindingList& self = GetInstance();
  self.Lock.Lock();

  RaverieForEach (BoundType* type, self.AllNativeBoundTypes)
  {
    type->IsInitializedAssert();
  }

  self.Lock.Unlock();
#endif
}

BoundType* NoType::RaverieGetDerivedType() const
{
  return nullptr;
}

bool TypeBinding::VirtualTableCounter::StaticDebugIsVirtual = false;

TypeBinding::VirtualTableCounter::VirtualTableCounter()
{
  // Make sure both flags are set to false so that we don't trip the assert
  StaticDebugIsVirtual = false;
  this->InstanceDebugIsVirtual = false;
}

void TypeBinding::VirtualTableCounter::AssertIfNotVirtual()
{
  // Perform the error checking
  ErrorIf(StaticDebugIsVirtual == false || this->InstanceDebugIsVirtual == false, "Method being tested was not virtual!");

  // Reset our state back, just in case we use this again
  StaticDebugIsVirtual = false;
  this->InstanceDebugIsVirtual = false;
}

BoundType* TypeBinding::StaticTypeId<void>::GetType()
{
  return Core::GetInstance().VoidType;
}

BoundType* TypeBinding::StaticTypeId<NoType>::GetType()
{
  return nullptr;
}

BoundType* TypeBinding::StaticTypeId<NullPointerType>::GetType()
{
  return Core::GetInstance().NullType;
}

AnyType* TypeBinding::StaticTypeId<Any>::GetType()
{
  return Core::GetInstance().AnythingType;
}

DelegateType* TypeBinding::StaticTypeId<Delegate>::GetType()
{
  return Core::GetInstance().AnyDelegateType;
}

BoundType* TypeBinding::StaticTypeId<Handle>::GetType()
{
  return Core::GetInstance().AnyHandleType;
}

AutoGrabAllocatingType::AutoGrabAllocatingType()
{
  if (ExecutableState::CallingState != nullptr)
    this->Type = ExecutableState::CallingState->AllocatingType;
  else
    this->Type = nullptr;
}

bool BoundTypeHelperIsRawCastable(BoundType* fromType, BoundType* toType)
{
  return fromType->IsRawCastableTo(toType);
}

bool BoundTypeHelperIsInitialized(BoundType* type)
{
  return type->IsInitialized();
}

bool BoundTypeHelperIsInitializedAssert(BoundType* type)
{
  return type->IsInitializedAssert();
}

String BoundTypeHelperGetName(BoundType* type)
{
  return type->Name;
}

void LibraryBuilderHelperAddNativeBoundType(LibraryBuilder& builder, BoundType* type, BoundType* base, TypeCopyMode::Enum mode)
{
  return builder.AddNativeBoundType(type, base, mode);
}

void InitializeTypeHelper(StringParam originalName, BoundType* type, size_t size, size_t rawVirtualcount)
{
  String typeName = LibraryBuilder::FixIdentifier(originalName, TokenCheck::IsUpper | TokenCheck::SkipPastScopeResolution, '\0');
  type->Name = typeName;
  type->TemplateBaseName = typeName;
  type->Size = size;
  type->RawNativeVirtualCount = rawVirtualcount;
}

RaverieDefineExternalBaseType(Boolean, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(Boolean2, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(Boolean3, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(Boolean4, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(Byte, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(Integer, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(Integer2, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(Integer3, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(Integer4, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(Real, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(Real2, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(Real3, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(Real4, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(Quaternion, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(Real2x2, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(Real3x3, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(Real4x4, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(String, TypeCopyMode::ReferenceType, builder, type)
{
}
RaverieDefineExternalBaseType(DoubleReal, TypeCopyMode::ValueType, builder, type)
{
}
RaverieDefineExternalBaseType(DoubleInteger, TypeCopyMode::ValueType, builder, type)
{
}
} // namespace Raverie
