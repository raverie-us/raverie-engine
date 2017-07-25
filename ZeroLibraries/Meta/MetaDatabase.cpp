///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Joshua Claeys
/// Copyright 2016-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(FactoryModified);
  DefineEvent(MetaModified);
  DefineEvent(ObjectStructureModified);
}//namespace Events

ZilchDefineType(MetaTypeEvent, builder, type)
{

}

//-------------------------------------------------------------------------- Meta Serialize Property
ZilchDefineType(MetaSerializedProperty, builder, type)
{
}

//**************************************************************************************************
MetaSerializedProperty::MetaSerializedProperty()
{
  MetaDatabase::GetInstance()->mDefaults.PushBack(this);
}

//**************************************************************************************************
MetaSerializedProperty::MetaSerializedProperty(AnyParam defaultValue) :
  mDefault(defaultValue)
{
  MetaDatabase::GetInstance()->mDefaults.PushBack(this);
}

//**************************************************************************************************
MetaSerializedProperty::~MetaSerializedProperty()
{
  MetaPropertyDefaultsList::Unlink(this);
}

//**************************************************************************************************
BoundType* MetaDatabase::FindType(StringParam typeName)
{
  return GetInstance()->mTypeMap.FindValue(typeName, nullptr);
}

//**************************************************************************************************
void MetaDatabase::AddLibrary(LibraryParam library)
{
  forRange (BoundType* type, library->BoundTypes.Values())
  {
    BoundType* oldType = mTypeMap[type->Name];
    if (oldType)
    {
      if (oldType->HasAttribute(ObjectAttributes::cProxy))
      {
        RemoveLibrary(oldType->SourceLibrary);
        ErrorIf(mTypeMap[type->Name] != nullptr, "The proxy type should have been erased");
      }
      // Exception proxies can replace current types
      else if (type->HasAttribute(ObjectAttributes::cExceptionProxy) == false)
      {
        Error(
          "A bound type is replacing another by the same name, "
          "which should only be done in the case of proxies");
      }
    }

    mTypeMap[type->Name] = type;

    // Add the type to meta compositions if it belongs to one
    // e.g. add ZilchComponents to the ComponentTypes on CogMetaComposition
    forRange(BoundType* compositionType, mCompositionTypes.All())
    {
      MetaComposition* composition = compositionType->HasInherited<MetaComposition>();
      if(type->IsA(composition->mComponentType))
        composition->mComponentTypes.Insert(type->Name, type);
    }

    // Add all event types
    forRange(SendsEvent* sendsEvents, type->SendsEvents.All())
    {
      String eventName = sendsEvents->Name;

      // If the event already exists in the database skip it (can have duplicate sends event entries)
      if(mEventMap.ContainsKey(eventName))
        continue;

      // Add to the meta database
      mEventMap[eventName] = sendsEvents->SentType;
    }
  }

  mLibraries.PushBack(library);
}

//**************************************************************************************************
void MetaDatabase::AddNativeLibrary(LibraryParam library)
{
  AddLibrary(library);
  mNativeLibraries.PushBack(library);
}

//**************************************************************************************************
void MetaDatabase::RemoveLibrary(LibraryParam library)
{
  // Remove all types from our type map
  forRange(BoundType* type, library->BoundTypes.Values())
    mTypeMap.Erase(type->Name);

  forRange(BoundType* type, library->BoundTypes.Values())
  {
    // Remove the type from meta compositions if it belongs to one
    // e.g. remove ZilchComponents from the ComponentTypes on CogMetaComposition
    forRange(BoundType* compositionType, MetaDatabase::GetInstance()->mCompositionTypes.All())
    {
      MetaComposition* composition = compositionType->HasInherited<MetaComposition>();

      // Composition may be null if we're shutting down
      if(composition && type->IsA(composition->mComponentType))
        composition->mComponentTypes.Erase(type->Name);
    }

    // Remove all event types from the meta database
    forRange(SendsEvent* sendsEvents, type->SendsEvents.All())
    {
      String eventName = sendsEvents->Name;

      // Only remove events still present (can have duplicate sends event entries)
      if(mEventMap.ContainsKey(eventName))
        mEventMap.Erase(eventName);
    }
  }
}

//**************************************************************************************************
void MetaDatabase::AddAlternateName(StringParam name, BoundType* boundType)
{
  mTypeMap[name] = boundType;
}

//**************************************************************************************************
void MetaDatabase::ReleaseDefaults()
{
  forRange(MetaSerializedProperty& prop, mDefaults.All())
  {
    prop.mDefault = Any();
  }
}

}//namespace Zero
