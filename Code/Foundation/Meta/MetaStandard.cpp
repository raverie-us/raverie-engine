// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

// Ranges
ZilchDefineRange(MetaSelection::range);

// METAREFACTOR - move to platform meta library
ZilchDefineExternalBaseType(IpAddress, TypeCopyMode::ReferenceType, builder, type)
{
  type->CreatableInScript = true;

  ZeroBindDocumented();

  ZilchBindConstructor();
  ZilchBindConstructor(const IpAddress&);
  ZilchBindConstructor(StringParam, uint, Zero::InternetProtocol::Enum);
  ZilchBindConstructor(StringParam, uint);

  ZilchBindDestructor();

  ZilchBindMethod(Clear);

  ZilchBindCustomGetterProperty(IsValid);
  ZilchBindGetterProperty(InternetProtocol);
  ZilchBindGetterProperty(String);
  ZilchBindCustomGetterProperty(Hash);

  ZilchFullBindGetterSetter(builder,
                            type,
                            &Zero::IpAddress::GetHost,
                            (String(Zero::IpAddress::*)() const),
                            &Zero::IpAddress::SetHost,
                            (void (Zero::IpAddress::*)(StringParam)),
                            "Host");
  ZilchFullBindGetterSetter(builder,
                            type,
                            &Zero::IpAddress::GetPort,
                            (uint(Zero::IpAddress::*)() const),
                            &Zero::IpAddress::SetPort,
                            (void (Zero::IpAddress::*)(uint)),
                            "Port");

  ZilchBindGetterProperty(PortString);
}

// Enums
ZilchDefineEnum(SendsEvents);
ZilchDefineEnum(InternetProtocol);

ZeroDefineArrayType(Array<Revision>);

ZilchDefineStaticLibrary(MetaLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  ZilchInitializeRangeAs(MetaSelection::range, "MetaSelectionRange");

  // Enums
  ZilchInitializeEnum(SendsEvents);
  ZilchInitializeEnum(InternetProtocol);

  // This needs to be the first thing initialized
  ZilchInitializeType(Object);
  ZilchInitializeType(EventObject);

  // Basic handles
  ZilchInitializeType(ReferenceCountedEmpty);
  ZilchInitializeType(SafeId32);
  ZilchInitializeType(SafeId64);
  ZilchInitializeType(ThreadSafeId32);
  ZilchInitializeType(ThreadSafeId64);
  ZilchInitializeType(ReferenceCountedSafeId32);
  ZilchInitializeType(ReferenceCountedSafeId64);
  ZilchInitializeType(ReferenceCountedThreadSafeId32);
  ZilchInitializeType(ReferenceCountedThreadSafeId64);

  // Object handles
  ZilchInitializeType(ReferenceCountedObject);
  ZilchInitializeType(SafeId32Object);
  ZilchInitializeType(SafeId64Object);
  ZilchInitializeType(ThreadSafeId32Object);
  ZilchInitializeType(ThreadSafeId64Object);
  ZilchInitializeType(ReferenceCountedSafeId32Object);
  ZilchInitializeType(ReferenceCountedSafeId64Object);
  ZilchInitializeType(ReferenceCountedThreadSafeId32Object);
  ZilchInitializeType(ReferenceCountedThreadSafeId64Object);

  // EventObject handles
  ZilchInitializeType(ReferenceCountedEventObject);
  ZilchInitializeType(SafeId32EventObject);
  ZilchInitializeType(SafeId64EventObject);
  ZilchInitializeType(ThreadSafeId32EventObject);
  ZilchInitializeType(ThreadSafeId64EventObject);
  ZilchInitializeType(ReferenceCountedSafeId32EventObject);
  ZilchInitializeType(ReferenceCountedSafeId64EventObject);
  ZilchInitializeType(ReferenceCountedThreadSafeId32EventObject);
  ZilchInitializeType(ReferenceCountedThreadSafeId64EventObject);

  ZilchInitializeType(ThreadSafeReferenceCounted);

  // Meta Components
  ZilchInitializeType(MetaAttribute);
  ZilchInitializeType(CogComponentMeta);
  ZilchInitializeType(MetaOwner);
  ZilchInitializeType(MetaGroup);
  ZilchInitializeType(MetaCustomUi);
  ZilchInitializeType(MetaOperations);
  ZilchInitializeType(MetaPropertyFilter);
  ZilchInitializeType(MetaPropertyBasicFilter);
  ZilchInitializeType(MetaEditorGizmo);
  ZilchInitializeType(MetaDisplay);
  ZilchInitializeType(TypeNameDisplay);
  ZilchInitializeType(StringNameDisplay);
  ZilchInitializeType(MetaTransform);
  ZilchInitializeType(MetaPropertyRename);
  ZilchInitializeType(MetaShaderInput);
  ZilchInitializeType(EditorPropertyExtension);
  ZilchInitializeType(EditorIndexedStringArray);
  ZilchInitializeType(EditorRange);
  ZilchInitializeType(EditorSlider);
  ZilchInitializeType(EditorRotationBasis);
  ZilchInitializeType(MetaEditorResource);
  ZilchInitializeType(MetaDataInheritance);
  ZilchInitializeType(MetaDataInheritanceRoot);
  ZilchInitializeType(MetaSerializedProperty);
  ZilchInitializeType(MetaComposition);
  ZilchInitializeType(MetaArray);
  ZilchInitializeType(MetaArrayWrapper);

  // Events
  ZilchInitializeType(Event);
  ZilchInitializeType(MetaLibraryEvent);
  ZilchInitializeType(SelectionChangedEvent);
  ZilchInitializeType(NotifyEvent);
  ZilchInitializeType(PropertyEvent);
  ZilchInitializeType(TypeEvent);

  ZilchInitializeType(MetaSelection);
  ZilchInitializeType(ObjectEvent);
  ZilchInitializeType(LocalModifications);
  ZilchInitializeType(PropertyPath);
  ZilchInitializeExternalType(IpAddress);

  ZilchInitializeType(PropertyPath);
  ZilchInitializeType(Revision);

  ZeroInitializeArrayTypeAs(Array<Revision>, "Revisions");

  MetaLibraryExtensions::AddNativeExtensions(builder);
}

void MetaLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  AttributeExtensions::Initialize();

  RegisterFunctionAttribute(Zilch::StaticAttribute)->AllowStatic(true);
  RegisterFunctionAttribute(Zilch::VirtualAttribute);
  RegisterFunctionAttribute(Zilch::OverrideAttribute);
  RegisterFunctionAttribute(PropertyAttributes::cInternal);
  RegisterFunctionAttribute(FunctionAttributes::cDisplay)->AllowStatic(true);

  RegisterPropertyAttribute(Zilch::StaticAttribute)->AllowStatic(true);
  RegisterPropertyAttribute(Zilch::VirtualAttribute);
  RegisterPropertyAttribute(Zilch::OverrideAttribute);
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

} // namespace Zero
