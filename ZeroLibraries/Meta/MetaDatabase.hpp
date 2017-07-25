///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Joshua Claeys
/// Copyright 2016-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
namespace Events
{
  DeclareEvent(MetaModified);
}//namespace Events


 //---------------------------------------------------------------------------------- Meta Type Event
class MetaTypeEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  BoundType* Type;
};

//-------------------------------------------------------------------------- Meta Serialize Property
class MetaSerializedProperty : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MetaSerializedProperty();
  MetaSerializedProperty(AnyParam defaultValue);
  ~MetaSerializedProperty();

  Any mDefault;

  IntrusiveLink(MetaSerializedProperty, mLink);
};

typedef InList<MetaSerializedProperty, &MetaSerializedProperty::mLink> MetaPropertyDefaultsList;

//------------------------------------------------------------------------------------ Meta Database
class MetaDatabase : public ExplicitSingleton<MetaDatabase, EventObject>
{
public:
  /// Find a meta type object by name.
  static BoundType* FindType(StringParam typeName);

  void AddLibrary(LibraryParam library);
  void AddNativeLibrary(LibraryParam library);
  void RemoveLibrary(LibraryParam library);
  void AddAlternateName(StringParam name, BoundType* boundType);

  void ReleaseDefaults();

  MetaPropertyDefaultsList mDefaults;

  typedef HashMap<String, BoundType*> StringToTypeMap;
  StringToTypeMap mEventMap;
  StringToTypeMap mTypeMap;
  Array<LibraryRef> mLibraries;
  Array<LibraryRef> mNativeLibraries;

  // Currently, this assumes all Composition types are native, therefor it's okay to store
  // a direct pointer. Should maybe change this.
  Array<BoundType*> mCompositionTypes;
};

}//namespace Zero
