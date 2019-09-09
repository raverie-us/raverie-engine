// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
namespace Events
{
DeclareEvent(MetaModified);
DeclareEvent(MetaRemoved);
} // namespace Events

// Meta Type Event
class MetaLibraryEvent : public Event
{
public:
  ZilchDeclareType(MetaLibraryEvent, TypeCopyMode::ReferenceType);
  LibraryRef mLibrary;
};

// Meta Serialize Property
class MetaSerializedProperty : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(MetaSerializedProperty, TypeCopyMode::ReferenceType);

  MetaSerializedProperty();
  MetaSerializedProperty(AnyParam defaultValue);
  ~MetaSerializedProperty();

  Any mDefault;

  IntrusiveLink(MetaSerializedProperty, mLink);
};

typedef InList<MetaSerializedProperty, &MetaSerializedProperty::mLink> MetaPropertyDefaultsList;

// Meta Database
class MetaDatabase : public ExplicitSingleton<MetaDatabase, EventObject>
{
public:
  /// Find a meta type object by name.
  static BoundType* FindType(StringParam typeName);

  /// We should only send events for script libraries, not native libraries.
  void AddLibrary(LibraryParam library, bool sendModifiedEvent = false);
  void AddNativeLibrary(LibraryParam library);
  void RemoveLibrary(LibraryParam library);
  void AddAlternateName(StringParam name, BoundType* boundType);

  void ReleaseDefaults();

  void ClearRemovedLibraries();

  MetaPropertyDefaultsList mDefaults;

  typedef HashMap<String, BoundType*> StringToTypeMap;
  StringToTypeMap mEventMap;
  StringToTypeMap mTypeMap;
  Array<LibraryRef> mLibraries;
  Array<LibraryRef> mNativeLibraries;

  // There is a time where new libraries should be alive at the same time as old
  // libraries that are being replaced During this time, we patch components
  // with their new types. We must keep the old removed libraries until we're
  // done fully patching. After we can call ClearRemovedLibraries to release the
  // final reference count
  Array<LibraryRef> mRemovedLibraries;

  // Currently, this assumes all Composition types are native, therefor it's
  // okay to store a direct pointer. Should maybe change this.
  Array<BoundType*> mCompositionTypes;
};

} // namespace Zero
