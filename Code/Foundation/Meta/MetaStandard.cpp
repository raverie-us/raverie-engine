// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

// Ranges
RaverieDefineRange(MetaSelection::range);

// METAREFACTOR - move to platform meta library
RaverieDefineExternalBaseType(IpAddress, TypeCopyMode::ReferenceType, builder, type)
{
  type->CreatableInScript = true;

  RaverieBindDocumented();

  RaverieBindConstructor();
  RaverieBindConstructor(const IpAddress&);
  RaverieBindConstructor(StringParam, uint, Raverie::InternetProtocol::Enum);
  RaverieBindConstructor(StringParam, uint);

  RaverieBindDestructor();

  RaverieBindMethod(Clear);

  RaverieBindCustomGetterProperty(IsValid);
  RaverieBindGetterProperty(InternetProtocol);
  RaverieBindGetterProperty(String);
  RaverieBindCustomGetterProperty(Hash);

  RaverieFullBindGetterSetter(builder,
                            type,
                            &Raverie::IpAddress::GetHost,
                            (String(Raverie::IpAddress::*)() const),
                            &Raverie::IpAddress::SetHost,
                            (void (Raverie::IpAddress::*)(StringParam)),
                            "Host");
  RaverieFullBindGetterSetter(builder,
                            type,
                            &Raverie::IpAddress::GetPort,
                            (uint(Raverie::IpAddress::*)() const),
                            &Raverie::IpAddress::SetPort,
                            (void (Raverie::IpAddress::*)(uint)),
                            "Port");

  RaverieBindGetterProperty(PortString);
}

// Enums
RaverieDefineEnum(SendsEvents);
RaverieDefineEnum(InternetProtocol);

RaverieDefineStaticLibrary(MetaLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  RaverieInitializeRangeAs(MetaSelection::range, "MetaSelectionRange");

  // Enums
  RaverieInitializeEnum(SendsEvents);
  RaverieInitializeEnum(InternetProtocol);

  // This needs to be the first thing initialized
  RaverieInitializeType(Object);
  RaverieInitializeType(EventObject);

  // Basic handles
  RaverieInitializeType(ReferenceCountedEmpty);
  RaverieInitializeType(SafeId32);
  RaverieInitializeType(SafeId64);
  RaverieInitializeType(ThreadSafeId32);
  RaverieInitializeType(ThreadSafeId64);
  RaverieInitializeType(ReferenceCountedSafeId32);
  RaverieInitializeType(ReferenceCountedSafeId64);
  RaverieInitializeType(ReferenceCountedThreadSafeId32);
  RaverieInitializeType(ReferenceCountedThreadSafeId64);

  // Object handles
  RaverieInitializeType(ReferenceCountedObject);
  RaverieInitializeType(SafeId32Object);
  RaverieInitializeType(SafeId64Object);
  RaverieInitializeType(ThreadSafeId32Object);
  RaverieInitializeType(ThreadSafeId64Object);
  RaverieInitializeType(ReferenceCountedSafeId32Object);
  RaverieInitializeType(ReferenceCountedSafeId64Object);
  RaverieInitializeType(ReferenceCountedThreadSafeId32Object);
  RaverieInitializeType(ReferenceCountedThreadSafeId64Object);

  // EventObject handles
  RaverieInitializeType(ReferenceCountedEventObject);
  RaverieInitializeType(SafeId32EventObject);
  RaverieInitializeType(SafeId64EventObject);
  RaverieInitializeType(ThreadSafeId32EventObject);
  RaverieInitializeType(ThreadSafeId64EventObject);
  RaverieInitializeType(ReferenceCountedSafeId32EventObject);
  RaverieInitializeType(ReferenceCountedSafeId64EventObject);
  RaverieInitializeType(ReferenceCountedThreadSafeId32EventObject);
  RaverieInitializeType(ReferenceCountedThreadSafeId64EventObject);

  RaverieInitializeType(ThreadSafeReferenceCounted);

  // Meta Components
  RaverieInitializeType(MetaAttribute);
  RaverieInitializeType(CogComponentMeta);
  RaverieInitializeType(MetaOwner);
  RaverieInitializeType(MetaGroup);
  RaverieInitializeType(MetaCustomUi);
  RaverieInitializeType(MetaOperations);
  RaverieInitializeType(MetaPropertyFilter);
  RaverieInitializeType(MetaPropertyBasicFilter);
  RaverieInitializeType(MetaEditorGizmo);
  RaverieInitializeType(MetaDisplay);
  RaverieInitializeType(TypeNameDisplay);
  RaverieInitializeType(StringNameDisplay);
  RaverieInitializeType(MetaTransform);
  RaverieInitializeType(MetaPropertyRename);
  RaverieInitializeType(MetaShaderInput);
  RaverieInitializeType(EditorPropertyExtension);
  RaverieInitializeType(EditorIndexedStringArray);
  RaverieInitializeType(EditorRange);
  RaverieInitializeType(EditorSlider);
  RaverieInitializeType(EditorRotationBasis);
  RaverieInitializeType(MetaEditorResource);
  RaverieInitializeType(MetaDataInheritance);
  RaverieInitializeType(MetaDataInheritanceRoot);
  RaverieInitializeType(MetaSerializedProperty);
  RaverieInitializeType(MetaComposition);
  RaverieInitializeType(MetaArray);
  RaverieInitializeType(MetaArrayWrapper);

  // Events
  RaverieInitializeType(Event);
  RaverieInitializeType(MetaLibraryEvent);
  RaverieInitializeType(SelectionChangedEvent);
  RaverieInitializeType(NotifyEvent);
  RaverieInitializeType(PropertyEvent);
  RaverieInitializeType(TypeEvent);

  RaverieInitializeType(MetaSelection);
  RaverieInitializeType(ObjectEvent);
  RaverieInitializeType(LocalModifications);
  RaverieInitializeType(PropertyPath);
  RaverieInitializeExternalType(IpAddress);

  RaverieInitializeType(PropertyPath);

  MetaLibraryExtensions::AddNativeExtensions(builder);
}

void MetaLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  AttributeExtensions::Initialize();

  RegisterFunctionAttribute(Raverie::StaticAttribute)->AllowStatic(true);
  RegisterFunctionAttribute(Raverie::VirtualAttribute);
  RegisterFunctionAttribute(Raverie::OverrideAttribute);
  RegisterFunctionAttribute(PropertyAttributes::cInternal);
  RegisterFunctionAttribute(FunctionAttributes::cDisplay)->AllowStatic(true);

  RegisterPropertyAttribute(Raverie::StaticAttribute)->AllowStatic(true);
  RegisterPropertyAttribute(Raverie::VirtualAttribute);
  RegisterPropertyAttribute(Raverie::OverrideAttribute);
  RegisterPropertyAttribute(PropertyAttributes::cProperty);
  RegisterPropertyAttribute(PropertyAttributes::cInternal);
  RegisterPropertyAttribute(PropertyAttributes::cSerialize);
  RegisterPropertyAttribute(PropertyAttributes::cDeprecatedSerialized);
  RegisterPropertyAttribute(PropertyAttributes::cDisplay)->AllowStatic(true);
  RegisterPropertyAttribute(PropertyAttributes::cDeprecatedEditable)->AllowStatic(true);
  RegisterPropertyAttribute(PropertyAttributes::cRuntimeClone)->AllowStatic(true);
  RegisterPropertyAttributeType(PropertyAttributes::cShaderInput, MetaShaderInput)->AllowMultiple(true);
  RegisterPropertyAttributeType(PropertyAttributes::cRenamedFrom, MetaPropertyRename);
  RegisterPropertyAttribute(PropertyAttributes::cLocalModificationOverride);
  RegisterPropertyAttributeType(PropertyAttributes::cGroup, MetaGroup)->AllowStatic(true);
  RegisterPropertyAttributeType(PropertyAttributes::cRange, EditorRange)->TypeMustBe(float)->AllowStatic(true);
  RegisterPropertyAttributeType(PropertyAttributes::cSlider, EditorSlider)->TypeMustBe(float)->AllowStatic(true);
}

void MetaLibrary::Shutdown()
{
  AttributeExtensions::Destroy();

  GetLibrary()->ClearComponents();
}

} // namespace Raverie
