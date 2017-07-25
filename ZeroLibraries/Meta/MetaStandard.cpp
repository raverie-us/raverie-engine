///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

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

  ZilchFullBindGetterSetter(builder, type,
    &Zero::IpAddress::GetHost, (String(Zero::IpAddress::*)() const),
    &Zero::IpAddress::SetHost, (void(Zero::IpAddress::*)(StringParam)),
    "Host");
  ZilchFullBindGetterSetter(builder, type,
    &Zero::IpAddress::GetPort, (uint(Zero::IpAddress::*)() const),
    &Zero::IpAddress::SetPort, (void(Zero::IpAddress::*)(uint)),
    "Port");

  ZilchBindGetterProperty(PortString);
}

// Enums
ZilchDefineEnum(SendsEvents);
ZilchDefineEnum(InternetProtocol);

ZeroDefineArrayType(Array<Revision>);

//**************************************************************************************************
ZilchDefineStaticLibrary(MetaLibrary)
{
  builder.CreatableInScriptDefault = false;
  
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
  ZilchInitializeType(CogComponentMeta);
  ZilchInitializeType(MetaOwner);
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
  ZilchInitializeType(EditorPropertyExtension);
  ZilchInitializeType(EditorIndexedStringArray);
  ZilchInitializeType(EditorRange);
  ZilchInitializeType(EditorRotationBasis);
  ZilchInitializeType(EditorResource);
  ZilchInitializeType(MetaDataInheritance);
  ZilchInitializeType(MetaDataInheritanceRoot);
  ZilchInitializeType(MetaSerializedProperty);
  ZilchInitializeType(MetaComposition);
  ZilchInitializeType(MetaArray);
  ZilchInitializeType(MetaArrayWrapper);

  // Events
  ZilchInitializeType(Event);
  ZilchInitializeType(MetaTypeEvent);
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

//**************************************************************************************************
void MetaLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
}

//**************************************************************************************************
void MetaLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

}// namespace Zero
