///////////////////////////////////////////////////////////////////////////////
///
/// \file Documentation.cpp
/// Declaration of the Documentation classes.
///
/// Authors: Chris Peters and Joshua Shlemmer
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(DocumentationLibrary, builder, type)
{
}

/////////////////////////////////////////////pl///////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////
String ReplaceTypeIfTemplated(StringParam typeString)
{
  if (typeString.Contains("["))
  {
    return BuildString(typeString.SubString(typeString.Begin(), typeString.FindFirstOf("[").Begin()), "[T]");
  }
  else
  {
    return typeString;
  }
}

MethodDoc *MethodDocWithSameParams(Array<MethodDoc*>& methodList, Function* function)
{
  uint matchIndex = (uint)-1;

  DelegateType* delegateType = function->FunctionType;

  // iterate over each method overload
  for (uint i = 0; i < methodList.Size(); ++i)
  {
    // if we don't even have the correct number of parameters, continue
    if (methodList[i]->mParameterList.Size() != delegateType->Parameters.Size())
      continue;

    MethodDoc* posMatchMeth = methodList[i];

    // iterate over all parameters and assume we found a match until we have a parameter mismatch
    bool match = true;
    for (uint j = 0; j < posMatchMeth->mParameterList.Size(); ++j)
    {
      if (posMatchMeth->mParameterList[j]->mType != delegateType->Parameters[j].ParameterType->ToString())
      {
        match = false;
        break;
      }
    }

    // if we found a match, set match index to a valid index and stop iterating
    if (match)
    {
      matchIndex = i;
      break;
    }
  }

  // if index is not a valid location, we did not find a match
  if (matchIndex == (uint)-1)
    return nullptr;

  // return matching method
  return methodList[matchIndex];
}

template<typename T>
bool TrimCompareFn(const T* lhs, const T* rhs)
{
  return lhs->mName < rhs->mName;
}

////////////////////////////////////////////////////////////////////////
// ExceptionDoc
////////////////////////////////////////////////////////////////////////
template<> struct Zero::Serialization::Trait<ExceptionDoc>
{
  enum { Type = StructureType::Object };
  static inline cstr TypeName() { return "Exception"; }
};

void ExceptionDoc::Serialize(Serializer& stream)
{
  SerializeName(mTitle);
  SerializeName(mMessage);
}

////////////////////////////////////////////////////////////////////////
// CommandDoc
////////////////////////////////////////////////////////////////////////
template<> struct Zero::Serialization::Trait<CommandDoc>
{

  enum { Type = StructureType::Object };
  static inline cstr TypeName() { return "Command"; }
};

void CommandDoc::Serialize(Serializer& stream)
{
  SerializeName(mName);
  SerializeName(mDescription);
  SerializeName(mShortcut);
  SerializeName(mTags);
}

////////////////////////////////////////////////////////////////////////
// CommandDocList
////////////////////////////////////////////////////////////////////////
template<> struct Zero::Serialization::Trait<CommandDocList>
{

  enum { Type = StructureType::Object };
  static inline cstr TypeName() { return "CommandList"; }
};

bool CommandCompareFn(CommandDoc *lhs, CommandDoc *rhs)
{
  return lhs->mName < rhs->mName;
}

void CommandDocList::Sort(void)
{
  Zero::Sort(mCommands.All(), CommandCompareFn);
}

void CommandDocList::Serialize(Serializer& stream)
{
  SerializeName(mCommands);
}

////////////////////////////////////////////////////////////////////////
// EventDoc
////////////////////////////////////////////////////////////////////////
template<> struct Zero::Serialization::Trait<EventDoc>
{

  enum { Type = StructureType::Object };
  static inline cstr TypeName() { return "Event"; }
};

EventDoc::EventDoc(StringParam eventType, StringParam eventName)
  : mType(eventType)
  , mName(eventName)
{
}
void EventDoc::Serialize(Serializer& stream)
{
  SerializeName(mName);
  SerializeName(mType);
  SerializeName(mSenders);
  SerializeName(mListeners);
}

bool EventDoc::operator<(const EventDoc& rhs) const
{
  if (mType == rhs.mType)
  {
    return mName < rhs.mName;
  }
  return mType < rhs.mType;
}

////////////////////////////////////////////////////////////////////////
// EventDocList
////////////////////////////////////////////////////////////////////////
template<> struct Zero::Serialization::Trait<EventDocList>
{

  enum { Type = StructureType::Object };
  static inline cstr TypeName() { return "EventList"; }
};

void EventDocList::Serialize(Serializer& stream)
{
  SerializeName(mEvents);
}

bool eventCompareFn(EventDoc* lhs, EventDoc* rhs)
{
  return *lhs < *rhs;
}

void EventDocList::Sort(void)
{
  Zero::Sort(mEvents.All(), eventCompareFn);
}

void EventDocList::BuildMap(void)
{
  mEventMap.Clear();

  for (uint i = 0; i < mEvents.Size(); ++i)
  {
    mEventMap[mEvents[i]->mName] = mEvents[i];
  }
}


////////////////////////////////////////////////////////////////////////
// PropertyDoc
////////////////////////////////////////////////////////////////////////=
template<> struct Zero::Serialization::Trait<PropertyDoc>
{

  enum { Type = StructureType::Object };
  static inline cstr TypeName() { return "Property"; }
};

void PropertyDoc::Serialize(Serializer& stream)
{
  SerializeName(mName);
  SerializeName(mType);
  SerializeName(mDescription);
  SerializeName(mReadOnly);
  SerializeName(mStatic);
}

////////////////////////////////////////////////////////////////////////
// ParameterDoc
////////////////////////////////////////////////////////////////////////
template<> struct Zero::Serialization::Trait<ParameterDoc>
{
  enum { Type = StructureType::Object };
  static inline cstr TypeName() { return "Parameter"; }
};

void ParameterDoc::Serialize(Serializer& stream)
{
  SerializeName(mName);
  SerializeName(mType);
  SerializeName(mDescription);

}

////////////////////////////////////////////////////////////////////////
// MethodDoc
////////////////////////////////////////////////////////////////////////
template<> struct Zero::Serialization::Trait<MethodDoc>
{

  enum { Type = StructureType::Object };
  static inline cstr TypeName() { return "Method"; }
};

MethodDoc::~MethodDoc()
{
  forRange(ParameterDoc *param, mParameterList.All())
  {
    delete param;
  }
}

void MethodDoc::Serialize(Serializer& stream)
{
  SerializeName(mName);
  SerializeName(mDescription);
  SerializeName(mReturnType);
  SerializeName(mParameters);
  SerializeName(mParameterList);
  SerializeName(mPossibleExceptionThrows);
  SerializeName(mStatic);
}

////////////////////////////////////////////////////////////////////////
// ClassDoc
////////////////////////////////////////////////////////////////////////
template<> struct Zero::Serialization::Trait<ClassDoc>
{

  enum { Type = StructureType::Object };
  static inline cstr TypeName() { return "ClassDoc"; }
};

ClassDoc::ClassDoc() : mBaseClassPtr(nullptr), mImportDocumentation(true)
{
}

ClassDoc::~ClassDoc()
{
  for (uint i = 0; i < mProperties.Size(); ++i)
  {
    delete mProperties[i];
  }

  for (uint i = 0; i < mMethods.Size(); ++i)
  {
    delete mMethods[i];
  }

  for (uint i = 0; i < mEventsSent.Size(); ++i)
  {
    delete mEventsSent[i];
  }
}

void ClassDoc::Serialize(Serializer& stream)
{
  SerializeName(mName);
  SerializeName(mBaseClass);
  SerializeName(mLibrary);
  SerializeName(mDescription);
  SerializeName(mTags);
  SerializeName(mProperties);
  SerializeName(mMethods);
  SerializeName(mEventsSent);
  SerializeName(mImportDocumentation);
}

void ClassDoc::BuildMapsAndArrays()
{
  // clear all the maps just in case we have some data floating in there
  mPropertiesMap.Clear();
  mMethodsMap.Clear();
  mEventsMap.Clear();

  // sort the arrays so when they serialize they will be alphabetical
  Zero::Sort(mProperties.All(), TrimCompareFn<PropertyDoc>);
  Zero::Sort(mMethods.All(), TrimCompareFn<MethodDoc>);
  Zero::Sort(mEventsSent.All(), TrimCompareFn<EventDoc>);

  // insert pointers into the arrays into the maps for the respective doc types

  for (uint i = 0; i < mProperties.Size(); ++i)
  {
    PropertyDoc* prop = mProperties[i];
    mPropertiesMap[prop->mName] = prop;
  }

  for (uint i = 0; i < mMethods.Size(); ++i)
  {
    MethodDoc* meth = mMethods[i];
    mMethodsMap[meth->mName].PushBack(meth);
  }

  for (uint i = 0; i < mEventsSent.Size(); ++i)
  {
    EventDoc* event = mEventsSent[i];
    mEventsMap[event->mName] = event;
  }
}

void ClassDoc::FillDocumentation(BoundType* classType)
{
  // METAREFACTOR we need to walk all extension functions and add documentation

  // Class description
  classType->Description = mDescription;

  // Properties
  forRange(Property* metaProperty, classType->GetProperties())
  {
    if(PropertyDoc* propertyDoc = GetPropertyDoc(metaProperty->Name))
      metaProperty->Description = propertyDoc->mDescription;
  }

  // METAREFACTOR Handle looking up overloads

  // Methods
  forRange(Function* function, classType->GetFunctions())
  {
    if(MethodDoc* methodDoc = GetMethodDoc(function))
    {
      // Function description
      function->Description = methodDoc->mDescription;

      // Parameter descriptions
      MethodDoc::ParameterArray& parameters = methodDoc->mParameterList;
      for(uint i = 0; i < parameters.Size(); ++i)
        function->ParameterDescriptions.PushBack(parameters[i]->mDescription);
    }
  }
}

PropertyDoc* ClassDoc::GetPropertyDoc(StringParam propertyName)
{
  return mPropertiesMap.FindValue(propertyName, nullptr);
}

MethodDoc* ClassDoc::GetMethodDoc(Function* function)
{
  if(MethodDocArray* overloadedMethods = mMethodsMap.FindPointer(function->Name))
  {
    // get the list of parameters so we can make sure we get the same function signature
    ParameterArray& functionParameters = function->FunctionType->Parameters;

    // Walk all overloaded methods and find the matching function
    forRange(MethodDoc* methodDoc, overloadedMethods->All())
    {
      MethodDoc::ParameterArray& docParameters = methodDoc->mParameterList;

      if (docParameters.Size() != functionParameters.Size())
        continue;

      bool matching = true;
      // Check to see if they have the same parameters
      for(uint i = 0; i < functionParameters.Size(); ++i)
      {
        DelegateParameter& functionParameter = functionParameters[i];
        ParameterDoc* docParameter = docParameters[i];

        // we have to make sure the docParameter type is in the same form that documentation uses
        String metaParamType = ReplaceTypeIfTemplated(functionParameter.ParameterType->ToString());

        if(metaParamType != docParameter->mType)
        {
          matching = false;
          break;
        }
      }

      if (matching)
      {
        for (uint i = 0; i < functionParameters.Size(); ++i)
        {
          functionParameters[i].Name = docParameters[i]->mName;
        }

        return methodDoc;
      }
    }
  }

  return nullptr;
}

////////////////////////////////////////////////////////////////////////
// EnumDoc
////////////////////////////////////////////////////////////////////////

void EnumDoc::Serialize(Serializer& stream)
{
  SerializeName(mName);
  SerializeName(mEnumValues);
  SerializeName(mDescription);
}

void EnumDoc::FillDocumentation(BoundType* enumOrFlagType)
{
  enumOrFlagType->Description = mDescription;

  // typedef used due to template type causing forRange macro to fail
  typedef Pair<String, String> pairType;

  // get the properties for enum values and fill their descriptions
  forRange(pairType& valueDescriptionPair, mEnumValues.All())
  {
    int enumValue = enumOrFlagType->StringToEnumValue[valueDescriptionPair.first];
    Array<Property*>& valueProp = enumOrFlagType->EnumValueToProperties[enumValue];

    forRange(Property* enumValueProperty, valueProp.All())
    {
      // since we get properties by value, we have to find the property with the right name
      if (enumValueProperty->Name == valueDescriptionPair.first)
      {
        enumValueProperty->Description = valueDescriptionPair.second;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////
// DocumentationLibrary
////////////////////////////////////////////////////////////////////////
DocumentationLibrary::~DocumentationLibrary()
{
  DeleteObjectsInContainer(mClasses);
  DeleteObjectsInContainer(mEnums);
  DeleteObjectsInContainer(mFlags);
}

void DocumentationLibrary::Serialize(Serializer& stream)
{
  SerializeName(mClasses);
  SerializeName(mEnums);
  SerializeName(mFlags);
}

void DocumentationLibrary::LoadDocumentation(StringParam fileName)
{
  // will throw if unable to load file
  LoadFromDataFile(*this, fileName);

  // finalize sorts data, fills maps, and fills base class pointers
  FinalizeDocumentation();

  // save the global documentation
  Z::gDocumentation = this;

  MetaDatabase* meta = MetaDatabase::GetInstance();
  forRange(Library* lib, meta->mNativeLibraries.All())
  {
    BoundType* resourceType = ZilchTypeId(Resource);

    forRange(BoundType* type, lib->BoundTypes.Values())
    {
      if (type->HasAttribute(ObjectAttributes::cDocumented)
        || type->HasAttribute(ImportDocumentation))
      {
        // Add Descriptions to all types, methods, and properties
        if (ClassDoc* classDoc = GetClassDoc(type))
        {
          classDoc->FillDocumentation(type);
        }
      }
      // Add Description to enum and flag types
      else if (mEnumAndFlagMap.ContainsKey(type->Name))
      {
        mEnumAndFlagMap[type->Name]->FillDocumentation(type);
      }
    }
  }
}

// Finalize the documentation for saving/loading
void DocumentationLibrary::FinalizeDocumentation(void)
{
  // clean up any possible floating data in the maps
  mClassMap.Clear();
  mEnumAndFlagMap.Clear();

  Zero::Sort(mClasses.All(), TrimCompareFn<ClassDoc>);
  Zero::Sort(mEnums.All(), TrimCompareFn<EnumDoc>);
  Zero::Sort(mFlags.All(), TrimCompareFn<EnumDoc>);


  // build the enum and flag map
  forRange(EnumDoc* enumDoc, mEnums.All())
  {
    mEnumAndFlagMap[enumDoc->mName] = enumDoc;
  }
  forRange(EnumDoc* flagDoc, mFlags.All())
  {
    mEnumAndFlagMap[flagDoc->mName] = flagDoc;
  }

  // build the class map
  for (uint i = 0; i < mClasses.Size(); ++i)
  {
    ClassDoc* currDoc = mClasses[i];

    // build each classes maps and arrays
    currDoc->BuildMapsAndArrays();

    mClassMap[currDoc->mName] = currDoc;
  }

  // Now that the map is complete, loop again to set up base class pointers
  for (uint i = 0; i < mClasses.Size(); ++i)
  {
    ClassDoc* currDoc = mClasses[i];

    if (!currDoc->mBaseClass.Empty())
    {
      currDoc->mBaseClassPtr = mClassMap[currDoc->mBaseClass];
    }
  }
}


void DocumentationLibrary::LoadFromMeta()
{
  // First get the range of types from the MetaDatabase
  MetaDatabase* database = MetaDatabase::GetInstance();

  // Keep map of function pointers to check for stuff we have already found
  HashSet<Function*> functionSet;

  // Loop over every type
  forRange(BoundType* metaType, database->mTypeMap.Values())
  {
    // so we can check if we should export internal description of type
    bool exportDoc = metaType->HasAttribute(ExportDocumentation);

    // check for enum types
    if (metaType->SpecialType == SpecialType::Enumeration 
      || metaType->SpecialType == SpecialType::Flags)
    {
      // save the enum
      EnumDoc *newEnumDoc = new EnumDoc();

      newEnumDoc->mName = metaType->Name;

      if (exportDoc)
        newEnumDoc->mDescription = metaType->Description;

      // typedef used due to template type causing forRange macro to fail
      typedef Pair<Integer, StringArray> pairType;

      forRange(pairType &enumPair, metaType->EnumValueToStrings.All())
      {
        forRange(StringParam enumValueName, enumPair.second.All())
        {
          newEnumDoc->mEnumValues.FindOrInsert(enumValueName,"");
        }
      }

      // save flags and enums separately
      if (metaType->SpecialType == SpecialType::Flags)
        mFlags.PushBack(newEnumDoc);
      else
        mEnums.PushBack(newEnumDoc);
    }

    // If the type is not even documented and is not from a core library, just skip it
    Zilch::Library* owningLibrary = metaType->SourceLibrary;
    if (!exportDoc
      && !metaType->HasAttribute(ObjectAttributes::cDocumented)
      && !metaType->HasAttribute(ImportDocumentation))
    {
       continue;
    }

    // Used to remove a couple of core types that should not be documented
    if (metaType->Name.Contains(":"))
    {
      ZPrint("Warning: type '%s' is improperly named and unreferenceable.\n", metaType->Name);
      continue;
    }

    // check if we have to do template name replacement
    String name = ReplaceTypeIfTemplated(metaType->Name);

    ClassDoc* classDoc;

    // If we have already seen this class before, grab it from the map
    if (mClassMap.ContainsKey(name))
      classDoc = mClassMap[name];
    // Otherwise, create a new classdoc and add it to the array and map
    else
    {
      classDoc = new ClassDoc();
      classDoc->mName = name;

      classDoc->mLibrary = metaType->SourceLibrary->Name;

      // if we have a baseType, save it
      if (metaType->BaseType)
        classDoc->mBaseClass = metaType->BaseType->Name;

      mClasses.PushBack(classDoc);
      mClassMap[classDoc->mName] = classDoc;
    }

    // This is to tell the doc tool to only import doxygen descriptions if we had either of these
    classDoc->mImportDocumentation = metaType->HasAttribute(ObjectAttributes::cDocumented)
      || metaType->HasAttribute(ImportDocumentation);

    if (exportDoc)
      classDoc->mDescription = metaType->Description;

    forRange(CogComponentMeta* metaComponent, metaType->HasAll<CogComponentMeta>())
      classDoc->mTags.Assign(metaComponent->mTags.All());

   // Loop over all events this meta type sends and save them
   forRange(SendsEvent* eventSent, metaType->SendsEvents.All())
   {
     EventDoc* eventDoc;
     BoundType* eventType = Type::GetBoundType(eventSent->SentType);

     // If we have not seen this event before, create a new event doc
     if (!classDoc->mEventsMap.ContainsKey(eventSent->Name))
     {
       eventDoc = new EventDoc();

       eventDoc->mName = eventSent->Name;

       if (eventSent->SentType)
         eventDoc->mType = eventType->Name;

       classDoc->mEventsSent.PushBack(eventDoc);

       eventDoc->mSenders.PushBack(classDoc->mName);

       classDoc->mEventsMap[eventDoc->mName] = eventDoc;
     }
     // Otherwise, if we have it, check if it is missing data
     else
     {
       // Check if we are missing type information for this event
       if (eventDoc->mType == "")
       {
         // If we were missing the event type, see if the version we just found has it
         if (eventType)
           eventDoc->mType = eventType->Name;
       }
     }
   }
   // Get all methods
   forRange(Function* method, metaType->AllFunctions.All())
   {
     if (functionSet.Contains(method))
       continue;

     MethodDoc* newMethod = new MethodDoc();

     newMethod->mStatic = method->IsStatic;

     // Currently, the majority of methods contain angle brackets that we have to strip.
     if (method->Name.Contains("["))
       newMethod->mName = method->Name.SubString(method->Name.Begin() + 1, method->Name.End() - 1);
     else
       newMethod->mName = method->Name;

     if (exportDoc)
      newMethod->mDescription = method->Description;

     classDoc->mMethodsMap[method->Name].PushBack(newMethod);

     classDoc->mMethods.PushBack(newMethod);

     DelegateType* type = method->FunctionType;

     Zilch::Type* returnType = type->Return;

     if (returnType)
     {
       newMethod->mReturnType = ReplaceTypeIfTemplated(returnType->ToString());
     }

     forRange(DelegateParameter& param, type->GetParameters().All())
     {
       ParameterDoc* newParam = new ParameterDoc();

       newParam->mName = param.Name;
       newParam->mType = ReplaceTypeIfTemplated(param.ParameterType->ToString());

       newMethod->mParameterList.PushBack(newParam);
     }

     // Insert the new method into the map so we can make sure to check for repeats.
     functionSet.Insert(method);
   }

   // Get all properties
   forRange(Property* metaProperty, metaType->AllProperties.All())
   {
     // Skip this property if we have already seen it
     if (classDoc->mPropertiesMap.ContainsKey(metaProperty->Name))
     {
       continue;
     }

     PropertyDoc* newProp = new PropertyDoc();

     // if we have no set function, this is a read-only property
     if (metaProperty->IsReadOnly())
       newProp->mReadOnly = true;

     newProp->mStatic = metaProperty->IsStatic;

     newProp->mName = metaProperty->Name;

     if (exportDoc)
      newProp->mDescription = metaProperty->Description;

     newProp->mType = ReplaceTypeIfTemplated(metaProperty->PropertyType->ToString());

     classDoc->mProperties.PushBack(newProp);

     classDoc->mPropertiesMap[newProp->mName] = newProp;
   }

  }
}

ClassDoc* DocumentationLibrary::GetClassDoc(BoundType* type)
{
  String name = ReplaceTypeIfTemplated(type->Name);
  return GetClassDoc(name);
}

ClassDoc* DocumentationLibrary::GetClassDoc(StringParam className)
{
  return mClassMap.FindValue(className, nullptr);
}


String GenerateDocumentationString(StringParam name)
{
  StringBuilder builder;
  // if we cannot find the class by this name, just return empty string
  if (!Z::gDocumentation->mClassMap.ContainsKey(name))
    return String();

  ClassDoc* classDoc = Z::gDocumentation->mClassMap[name];

  builder << "Class " << classDoc->mName << "\n";
  builder << "Description: " << classDoc->mDescription << "\n\n";

  builder << "Properties: \n\n";
  if (!classDoc->mProperties.Empty())
  {
    forRange(PropertyDoc* propertyDoc, classDoc->mProperties.All())
    {
      builder << propertyDoc->mName << " : " << propertyDoc->mType << "\n";
      builder << propertyDoc->mDescription << "\n";
      builder << "\n";
    }
  }

  if (!classDoc->mEventsSent.Empty())
  {
    builder << "Events: \n\n";
    forRange(EventDoc* eventDoc, classDoc->mEventsSent.All())
    {
      builder << "On: " << eventDoc->mName;
      builder << " Sends: " << eventDoc->mType << "\n";
    }
    builder << "\n";
  }

  if (!classDoc->mMethods.Empty())
  {
    builder << "Methods: \n\n";
    forRange(MethodDoc* methodDoc, classDoc->mMethods.All())
    {
      builder << methodDoc->mReturnType << " " << methodDoc->mName << methodDoc->mParameters << "\n";
      builder << methodDoc->mDescription << "\n";
      builder << "\n";
    }

  }
  return builder.ToString();
}

void SaveInfoFromMetaToFile(StringParam fileName, bool saveUnbound)
{
  DocumentationLibrary* newDoc = new DocumentationLibrary();
  newDoc->LoadFromMeta();
  SaveToDataFile(*newDoc, fileName);
  SafeDelete(newDoc);
}

void ShutdownDocumentation()
{
  SafeDelete(Z::gDocumentation);
}

namespace Z
{
  DocumentationLibrary* gDocumentation = nullptr;
}

}
