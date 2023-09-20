// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(DocumentationLibrary, builder, type)
{
}

// Helpers

// Note: returns empty string if this is a type we shouldn't document
String ReplaceTypeIfTemplated(StringParam typeString)
{
  // if the whole type is inside brackets, just remove the brackets,
  if (typeString.StartsWith("[") && typeString.EndsWith("]"))
  {
    return typeString.SubString(typeString.Begin() + 1, typeString.End() - 1);
  }
  else if (typeString.Contains("["))
  {
    if (!typeString.Contains("any") || !typeString.Contains("Type"))
    {
      return String();
    }
    return BuildString(typeString.SubString(typeString.Begin(), typeString.FindFirstOf("[").Begin()), "[T]");
  }
  else if (typeString == "any")
  {
    return "T";
  }
  else
  {
    return typeString;
  }
}

MethodDoc* MethodDocWithSameParams(Array<MethodDoc*>& methodList, Raverie::Function* function)
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

    // iterate over all parameters and assume we found a match until we have a
    // parameter mismatch
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

template <typename T>
bool TrimCompareFn(const T* lhs, const T* rhs)
{
  return lhs->mName.ToLower() < rhs->mName.ToLower();
}

template <>
bool TrimCompareFn(const MethodDoc* lhs, const MethodDoc* rhs)
{
  String lhsName = lhs->mName.ToLower();
  String rhsName = rhs->mName.ToLower();
  int nameComparison = lhsName.CompareTo(rhsName);

  if (nameComparison != 0)
  {
    return nameComparison < 0;
  }

  uint iterLimit = Math::Min(lhs->mParameterList.Size(), rhs->mParameterList.Size());

  // if the names are the same, sort by parameter type
  for (uint i = 0; i < iterLimit; ++i)
  {
    String lhsTypeName = lhs->mParameterList[i]->mType.ToLower();
    String rhsTypeName = rhs->mParameterList[i]->mType.ToLower();
    int typeComparison = lhsTypeName.CompareTo(rhsTypeName);

    if (typeComparison != 0)
      return typeComparison < 0;
  }

  // if we get here, all the param types up to min param count were the same
  return lhs->mParameterList.Size() < rhs->mParameterList.Size();
}

// creates the full template name but with the template type names instead of
// the instance type names
String BuildDocumentationFullTemplateName(StringParam baseName, Array<Constant>& templateArgs, TypeReplacementMap& replacements)
{
  String argument = templateArgs[0].ToString();

  StringBuilder builder;
  builder << baseName << "[" << ReplaceTypeIfOnList(argument, &replacements);

  for (uint32_t i = 1; i < templateArgs.Size(); ++i)
  {
    argument = templateArgs[i].ToString();

    builder << ", " << ReplaceTypeIfOnList(argument, &replacements);
  }

  builder << "]";
  return builder.ToString();
}

String ReplaceTypeIfOnList(String& type, TypeReplacementMap* replacements)
{
  String returnString = type;

  // if we have no replacements to search, just return the type
  if (replacements == nullptr)
    return type;

  // replace the type if it is in our replacements list
  if (replacements->FindPairPointer(type) != nullptr)
  {
    return replacements->FindPairPointer(type)->second;
  }

  // check if this type contains one of our replacements
  Tokenizer typeTokenizer;

  TempToken currToken;

  typeTokenizer.Load(type);

  while (!typeTokenizer.AtEnd())
  {
    typeTokenizer.ReadToken(currToken);

    auto* replacementPair = replacements->FindPairPointer(currToken.Text);

    if (!replacementPair)
      continue;

    // make sure we are not just appending more template params by replacing
    // base with full type
    if (type == replacementPair->second)
      return returnString;

    returnString = returnString.Replace(replacementPair->first, replacementPair->second);
  }

  return returnString;
}

// inserts new replacements into the the replacement map from template and
// returns full doc name
String InsertIntoReplacementsMap(
    String& baseName, String& fullInstanceName, InstantiateTemplateInfo& templateHandler, Array<Constant>& dummyTypes, ArrayMap<String, String>& replacements, String* fullDocName = nullptr)
{
  Array<TemplateParameter>& params = templateHandler.TemplateParameters;

  // maybe theway this has to wok is if we see a mismatch we have to do some
  // replacement shit
  if (params.Size() != dummyTypes.Size())
  {
    Error("Template Parameter and type replacement length mismatch, unable to "
          "document template.");
    return "";
  }

  // get the full name with the nice template param names, instead of the dummy
  // type names
  String fullDocTemplateName;

  if (fullDocName == nullptr)
  {
    StringBuilder fullNameBuilder;

    fullNameBuilder << baseName << "[" << params[0].Name;

    for (uint32_t i = 1; i < params.Size(); ++i)
    {
      fullNameBuilder << ", " << params[i].Name;
    }

    fullNameBuilder << "]";

    fullDocTemplateName = fullNameBuilder.ToString();
  }
  else
  {
    fullDocTemplateName = *fullDocName;
  }

  replacements.InsertOrAssign(baseName, fullDocTemplateName);
  replacements.InsertOrAssign(fullInstanceName, fullDocTemplateName);

  for (uint32_t i = 0; i < params.Size(); ++i)
  {
    replacements.InsertOrAssign(dummyTypes[i].ToString(), params[i].Name);
  }

  return fullDocTemplateName;
}

// ExceptionDoc
template <>
struct Raverie::Serialization::Trait<ExceptionDoc>
{
  enum
  {
    Type = StructureType::Object
  };
  static inline cstr TypeName()
  {
    return "Exception";
  }
};

void ExceptionDoc::Serialize(Serializer& stream)
{
  SerializeNameDefault(mTitle, String());
  SerializeNameDefault(mMessage, String());
}

// CommandDoc
template <>
struct Raverie::Serialization::Trait<CommandDoc>
{

  enum
  {
    Type = StructureType::Object
  };
  static inline cstr TypeName()
  {
    return "Command";
  }
};

void CommandDoc::Serialize(Serializer& stream)
{
  SerializeNameDefault(mName, String());
  SerializeNameDefault(mDescription, String());
  SerializeNameDefault(mShortcut, String());
  SerializeNameDefault(mTags, StringArray());
}

// CommandDocList
bool CommandCompareFn(CommandDoc* lhs, CommandDoc* rhs)
{
  return lhs->mName < rhs->mName;
}

void CommandDocList::Sort(void)
{
  Raverie::Sort(mCommands.All(), CommandCompareFn);
}

void CommandDocList::Serialize(Serializer& stream)
{
  SerializeNameDefault(mCommands, Array<CommandDoc*>());
}

// EventDoc
template <>
struct Raverie::Serialization::Trait<EventDoc>
{

  enum
  {
    Type = StructureType::Object
  };
  static inline cstr TypeName()
  {
    return "Event";
  }
};

EventDoc::EventDoc(StringParam eventType, StringParam eventName) : mType(eventType), mName(eventName)
{
}
void EventDoc::Serialize(Serializer& stream)
{
  SerializeNameDefault(mName, String());
  SerializeNameDefault(mType, String());
  SerializeNameDefault(mSenders, StringArray());
  SerializeNameDefault(mListeners, StringArray());
}

bool EventDoc::operator<(const EventDoc& rhs) const
{
  if (mType == rhs.mType)
  {
    return mName < rhs.mName;
  }
  return mType < rhs.mType;
}

// EventDocList
template <>
struct Raverie::Serialization::Trait<EventDocList>
{

  enum
  {
    Type = StructureType::Object
  };
  static inline cstr TypeName()
  {
    return "EventList";
  }
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
  Raverie::Sort(mEvents.All(), eventCompareFn);
}

void EventDocList::BuildMap(void)
{
  mEventMap.Clear();

  for (uint i = 0; i < mEvents.Size(); ++i)
  {
    mEventMap[mEvents[i]->mName] = mEvents[i];
  }
}

// PropertyDoc
template <>
struct Raverie::Serialization::Trait<PropertyDoc>
{

  enum
  {
    Type = StructureType::Object
  };
  static inline cstr TypeName()
  {
    return "Property";
  }
};

void PropertyDoc::Serialize(Serializer& stream)
{
  SerializeNameDefault(mName, String());
  SerializeNameDefault(mType, String());
  SerializeNameDefault(mDescription, String());
  SerializeNameDefault(mReadOnly, false);
  SerializeNameDefault(mStatic, false);
}

// ParameterDoc
template <>
struct Raverie::Serialization::Trait<ParameterDoc>
{
  enum
  {
    Type = StructureType::Object
  };
  static inline cstr TypeName()
  {
    return "Parameter";
  }
};

void ParameterDoc::Serialize(Serializer& stream)
{
  SerializeNameDefault(mName, String());
  SerializeNameDefault(mType, String());
  SerializeNameDefault(mDescription, String());
}

// MethodDoc
template <>
struct Raverie::Serialization::Trait<MethodDoc>
{

  enum
  {
    Type = StructureType::Object
  };
  static inline cstr TypeName()
  {
    return "Method";
  }
};

static const String cDefaultParameters("()");
static const String cDefaultReturn("Void");
MethodDoc::MethodDoc() : mStatic(false), mParameters(cDefaultParameters), mReturnType(cDefaultReturn)
{
}

MethodDoc::~MethodDoc()
{
  forRange (ParameterDoc* param, mParameterList.All())
  {
    delete param;
  }
  forRange (ExceptionDoc* except, mPossibleExceptionThrows.All())
  {
    delete except;
  }
}

void MethodDoc::Serialize(Serializer& stream)
{
  SerializeNameDefault(mName, String());
  SerializeNameDefault(mDescription, String());
  SerializeNameDefault(mReturnType, cDefaultReturn);
  SerializeNameDefault(mParameters, cDefaultParameters);
  SerializeNameDefault(mParameterList, Array<ParameterDoc*>());
  SerializeNameDefault(mPossibleExceptionThrows, Array<ExceptionDoc*>());
  SerializeNameDefault(mStatic, false);
}

// AttributeDoc
AttributeDoc::AttributeDoc(AttributeExtension* attribute)
{
  mName = attribute->mAttributeName;
  mAllowStatic = attribute->mAllowStatic;
  mAllowMultiple = attribute->mAllowMultiple;
  mDeveloperAttribute = false;
}

bool AttributeDocCompareFn(AttributeDoc* lhs, AttributeDoc* rhs)
{
  return lhs->mName < rhs->mName;
}

void AttributeDoc::Serialize(Serializer& stream)
{
  SerializeNameDefault(mName, String());
  SerializeNameDefault(mDescription, String());
  SerializeNameDefault(mAllowStatic, false);
  SerializeNameDefault(mAllowMultiple, false);
  SerializeNameDefault(mDeveloperAttribute, false);
}

// AttributeDocList
Raverie::AttributeDocList::~AttributeDocList()
{
  forRange (AttributeDoc* attribToDelete, mObjectAttributes.All())
  {
    delete attribToDelete;
  }
  forRange (AttributeDoc* attribToDelete, mFunctionAttributes.All())
  {
    delete attribToDelete;
  }
  forRange (AttributeDoc* attribToDelete, mPropertyAttributes.All())
  {
    delete attribToDelete;
  }
}

void AttributeDocList::Serialize(Serializer& stream)
{
  SerializeNameDefault(mObjectAttributes, Array<AttributeDoc*>());
  SerializeNameDefault(mFunctionAttributes, Array<AttributeDoc*>());
  SerializeNameDefault(mPropertyAttributes, Array<AttributeDoc*>());
}

// we have a seperate saveToFile since the doc tool can't use Serialize function
bool AttributeDocList::SaveToFile(StringParam fileName)
{
  Status status;
  TextSaver saver;

  saver.Open(status, fileName.c_str());

  if (status.Failed())
  {
    Error("Unable to save attribute list file: %s\n", fileName.c_str());
    return false;
  }

  saver.StartPolymorphic("AttributeDocList");

  saver.SerializeField("ObjectAttributes", mObjectAttributes);
  saver.SerializeField("FunctionAttributes", mFunctionAttributes);
  saver.SerializeField("PropertyAttributes", mPropertyAttributes);

  saver.EndPolymorphic();

  saver.Close();

  return true;
}

void AttributeDocList::CreateAttributeMap(void)
{
  forRange (AttributeDoc* attrib, mObjectAttributes.All())
  {
    mObjectAttributesMap[attrib->mName] = attrib;
  }
  forRange (AttributeDoc* attrib, mFunctionAttributes.All())
  {
    mFunctionAttributesMap[attrib->mName] = attrib;
  }
  forRange (AttributeDoc* attrib, mPropertyAttributes.All())
  {
    mPropertyAttributesMap[attrib->mName] = attrib;
  }
}

void AttributeDocList::Sort(void)
{
  Raverie::Sort(mObjectAttributes.All(), AttributeDocCompareFn);
  Raverie::Sort(mFunctionAttributes.All(), AttributeDocCompareFn);
  Raverie::Sort(mPropertyAttributes.All(), AttributeDocCompareFn);
}

// ClassDoc
template <>
struct Raverie::Serialization::Trait<ClassDoc>
{

  enum
  {
    Type = StructureType::Object
  };
  static inline cstr TypeName()
  {
    return "ClassDoc";
  }
};

ClassDoc::ClassDoc() : mBaseClassPtr(nullptr), mImportDocumentation(true), mDevOnly(false)
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
  SerializeNameDefault(mName, String());
  SerializeNameDefault(mBaseClass, String());
  SerializeNameDefault(mLibrary, String());
  SerializeNameDefault(mDescription, String());
  SerializeNameDefault(mTags, StringArray());
  SerializeNameDefault(mProperties, Array<PropertyDoc*>());
  SerializeNameDefault(mMethods, Array<MethodDoc*>());
  SerializeNameDefault(mEventsSent, Array<EventDoc*>());
  SerializeNameDefault(mImportDocumentation, false);
  SerializeNameDefault(mDevOnly, false);
}

void ClassDoc::BuildMapsAndArrays()
{
  // clear all the maps just in case we have some data floating in there
  mPropertiesMap.Clear();
  mMethodsMap.Clear();
  mEventsMap.Clear();

  // sort the arrays so when they serialize they will be alphabetical
  Raverie::Sort(mProperties.All(), TrimCompareFn<PropertyDoc>);
  Raverie::Sort(mMethods.All(), TrimCompareFn<MethodDoc>);
  Raverie::Sort(mEventsSent.All(), TrimCompareFn<EventDoc>);

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
  forRange (Property* metaProperty, classType->GetProperties())
  {
    if (PropertyDoc* propertyDoc = GetPropertyDoc(metaProperty->Name))
      metaProperty->Description = propertyDoc->mDescription;
  }

  // METAREFACTOR Handle looking up overloads

  // Methods
  forRange (Function* function, classType->GetFunctions())
  {
    if (MethodDoc* methodDoc = GetMethodDoc(function))
    {
      // Function description
      function->Description = methodDoc->mDescription;

      // Parameter descriptions
      MethodDoc::ParameterArray& parameters = methodDoc->mParameterList;
      for (uint i = 0; i < parameters.Size(); ++i)
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
  if (MethodDocArray* overloadedMethods = mMethodsMap.FindPointer(function->Name))
  {
    // get the list of parameters so we can make sure we get the same function
    // signature
    ParameterArray& functionParameters = function->FunctionType->Parameters;

    // Walk all overloaded methods and find the matching function
    forRange (MethodDoc* methodDoc, overloadedMethods->All())
    {
      MethodDoc::ParameterArray& docParameters = methodDoc->mParameterList;

      if (docParameters.Size() != functionParameters.Size())
        continue;

      bool matching = true;
      // Check to see if they have the same parameters
      for (uint i = 0; i < functionParameters.Size(); ++i)
      {
        DelegateParameter& functionParameter = functionParameters[i];
        ParameterDoc* docParameter = docParameters[i];

        // we have to make sure the docParameter type is in the same form that
        // documentation uses
        String metaParamType = ReplaceTypeIfTemplated(functionParameter.ParameterType->ToString());

        if (metaParamType != docParameter->mType)
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

void ClassDoc::CreateMethodDocFromBoundType(Raverie::Function* method, TypeReplacementMap* replacements, bool exportDoc)
{

  MethodDoc* newMethod = new MethodDoc();

  newMethod->mStatic = method->IsStatic;

  // Currently, the majority of methods contain angle brackets that we have to
  // strip.
  if (method->Name.StartsWith("["))
  {
    newMethod->mName = method->Name.SubString(method->Name.Begin() + 1, method->Name.End() - 1);
    if (newMethod->mName == "Constructor")
    {
      newMethod->mName = mName;
    }
  }
  else
    newMethod->mName = method->Name;

  if (exportDoc)
    newMethod->mDescription = method->Description;

  mMethodsMap[method->Name].PushBack(newMethod);

  mMethods.PushBack(newMethod);

  DelegateType* type = method->FunctionType;

  Raverie::Type* returnType = type->Return;

  if (returnType)
  {
    newMethod->mReturnType = ReplaceTypeIfTemplated(returnType->ToString());

    // if empty, this was a template type we shouldn't replace
    if (newMethod->mReturnType.Empty())
    {
      newMethod->mReturnType = returnType->ToString();
    }
    // replace the type if it is in our replacements list
    newMethod->mReturnType = ReplaceTypeIfOnList(newMethod->mReturnType, replacements);
  }

  forRange (DelegateParameter& param, type->GetParameters().All())
  {
    ParameterDoc* newParam = new ParameterDoc();

    newParam->mName = param.Name;
    newParam->mType = ReplaceTypeIfTemplated(param.ParameterType->ToString());

    // if empty, this was a template type we shouldn't replace
    if (newParam->mType.Empty())
      newParam->mType = param.ParameterType->ToString();

    // replace the type if it is in our replacements list
    newParam->mType = ReplaceTypeIfOnList(newParam->mType, replacements);

    newMethod->mParameterList.PushBack(newParam);
  }
}

void ClassDoc::CreatePropertyDocFromBoundType(Property* metaProperty, TypeReplacementMap* replacements, bool exportDoc)
{
  // Skip this property if we have already seen it
  if (mPropertiesMap.ContainsKey(metaProperty->Name))
  {
    return;
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

  // if empty, this was a template type we shouldn't replace
  if (newProp->mType.Empty())
    newProp->mType = metaProperty->PropertyType->ToString();

  // replace the type if it is in our replacements list
  newProp->mType = ReplaceTypeIfOnList(newProp->mType, replacements);

  mProperties.PushBack(newProp);

  mPropertiesMap[newProp->mName] = newProp;
}

// EnumDoc

void EnumDoc::Serialize(Serializer& stream)
{
  SerializeNameDefault(mName, String());
  SerializeNameDefault(mEnumValues, TypeReplacementMap());
  SerializeNameDefault(mDescription, String());
}

void EnumDoc::FillDocumentation(BoundType* enumOrFlagType)
{
  enumOrFlagType->Description = mDescription;

  // typedef used due to template type causing forRange macro to fail
  typedef Pair<String, String> pairType;

  // get the properties for enum values and fill their descriptions
  forRange (pairType& valueDescriptionPair, mEnumValues.All())
  {
    int enumValue = enumOrFlagType->StringToEnumValue[valueDescriptionPair.first];
    Array<Property*>& valueProp = enumOrFlagType->EnumValueToProperties[enumValue];

    forRange (Property* enumValueProperty, valueProp.All())
    {
      // since we get properties by value, we have to find the property with the
      // right name
      if (enumValueProperty->Name == valueDescriptionPair.first)
      {
        enumValueProperty->Description = valueDescriptionPair.second;
      }
    }
  }
}

// DocumentationLibrary
DocumentationLibrary::~DocumentationLibrary()
{
  DeleteObjectsInContainer(mClasses);
  DeleteObjectsInContainer(mEnums);
  DeleteObjectsInContainer(mFlags);
}

void DocumentationLibrary::Serialize(Serializer& stream)
{
  SerializeNameDefault(mClasses, Array<ClassDoc*>());
  SerializeNameDefault(mEnums, Array<EnumDoc*>());
  SerializeNameDefault(mFlags, Array<EnumDoc*>());
}

void DocumentationLibrary::LoadDocumentation(StringParam fileName)
{
  // save the global documentation
  Z::gDocumentation = this;

  // will throw if unable to load file
  LoadFromDataFile(*this, fileName);

  // finalize sorts data, fills maps, and fills base class pointers
  FinalizeDocumentation();

  MetaDatabase* meta = MetaDatabase::GetInstance();
  forRange (Library* lib, meta->mNativeLibraries.All())
  {
    BoundType* resourceType = RaverieTypeId(Resource);

    forRange (BoundType* type, lib->BoundTypes.Values())
    {
      ClassDoc* classDoc = GetClassDoc(type);
      if (classDoc)
      {
        // Add Descriptions to all types, methods, and properties
        classDoc->FillDocumentation(type);
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

  Raverie::Sort(mClasses.All(), TrimCompareFn<ClassDoc>);
  Raverie::Sort(mEnums.All(), TrimCompareFn<EnumDoc>);
  Raverie::Sort(mFlags.All(), TrimCompareFn<EnumDoc>);

  // build the enum and flag map
  forRange (EnumDoc* enumDoc, mEnums.All())
  {
    mEnumAndFlagMap[enumDoc->mName] = enumDoc;
  }
  forRange (EnumDoc* flagDoc, mFlags.All())
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

// due to not all templates filling out their base name, strip everything after
// to bracket to make our own
String GetTemplateBaseName(StringParam templateName)
{
  StringRange paramSubstring = templateName.FindFirstOf("[");

  if (paramSubstring.Empty())
    return templateName;

  return templateName.SubString(templateName.Begin(), paramSubstring.Begin());
}

void DocumentationLibrary::CreateFlagOrEnumDocFromBoundType(BoundType* type, bool exportDoc)
{
  // save the enum
  EnumDoc* newEnumDoc = new EnumDoc();

  newEnumDoc->mName = type->Name;

  if (exportDoc)
    newEnumDoc->mDescription = type->Description;

  // typedef used due to template type causing forRange macro to fail
  typedef Pair<Integer, StringArray> pairType;

  forRange (pairType& enumPair, type->EnumValueToStrings.All())
  {
    forRange (StringParam enumValueName, enumPair.second.All())
    {
      newEnumDoc->mEnumValues.FindOrInsert(enumValueName, "");
    }
  }

  // save flags and enums separately
  if (type->SpecialType == SpecialType::Flags)
    mFlags.PushBack(newEnumDoc);
  else
    mEnums.PushBack(newEnumDoc);
}

void ClassDoc::CreateEventDocFromBoundType(SendsEvent* eventSent)
{
  EventDoc* eventDoc = nullptr;
  BoundType* eventType = Type::GetBoundType(eventSent->SentType);

  // If we have not seen this event before, create a new event doc
  if (!mEventsMap.ContainsKey(eventSent->Name))
  {
    eventDoc = new EventDoc();

    eventDoc->mName = eventSent->Name;

    if (eventSent->SentType)
      eventDoc->mType = eventType->Name;

    mEventsSent.PushBack(eventDoc);

    eventDoc->mSenders.PushBack(mName);

    mEventsMap[eventDoc->mName] = eventDoc;
  }
  // Otherwise, if we have it, check if it is missing data
  else
  {
    // Check if we are missing type information for this event
    if (eventDoc->mType == "")
    {
      // If we were missing the event type, see if the version we just found has
      // it
      if (eventType)
        eventDoc->mType = eventType->Name;
    }
  }
}

ClassDoc* DocumentationLibrary::CreateClassDocFromBoundType(BoundType* type, TypeReplacementMap* replacements)
{
  HashSet<Function*> functionSet;

  // so we can check if we should export internal description of type
  bool exportDoc = type->HasAttribute(ExportDocumentation);

  // check for enum types
  if (type->IsEnumOrFlags())
  {
    CreateFlagOrEnumDocFromBoundType(type, exportDoc);
    return nullptr;
  }

  if (type->HasAttribute(ObjectAttributes::cDoNotDocument) || (!type->Location.IsNative && !exportDoc))
  {
    return nullptr;
  }

  // Used to remove a couple of core types that should not be documented
  if (type->Name.Contains(":"))
  {
    ZPrint("Warning: type '%s' is improperly named and unreferenceable.\n", type->Name.c_str());
    return nullptr;
  }

  // check if we have to do template name replacement
  String name = type->Name;

  String nonTemplatedName = name.SubString(name.Begin(), name.FindFirstOf("[").Begin());

  name = ReplaceTypeIfTemplated(type->Name);

  // either grab class we have seen before, or create a new one
  ClassDoc* classDoc;

  if (name.Empty())
  {
    if (!nonTemplatedName.Empty() && mClassMap.ContainsKey(nonTemplatedName))
    {
      classDoc = mClassMap[nonTemplatedName];
    }
    else
    {
      return nullptr;
    }
  }
  // If we have already seen this class before, grab it from the map
  else if (mClassMap.ContainsKey(name))
  {
    classDoc = mClassMap[name];
  }
  // Otherwise, create a new classdoc and add it to the array and map
  else
  {
    classDoc = new ClassDoc();
    classDoc->mName = name;

    classDoc->mLibrary = type->SourceLibrary->Name;

    // if we have a baseType, save it
    if (type->BaseType)
      classDoc->mBaseClass = ReplaceTypeIfTemplated(type->BaseType->Name);

    mClasses.PushBack(classDoc);
    mClassMap[classDoc->mName] = classDoc;
  }

  // This is to tell the doc tool to only import doxygen descriptions if we had
  // either of these
  classDoc->mImportDocumentation = type->HasAttribute(ObjectAttributes::cDocumented) || type->HasAttribute(ImportDocumentation);

  if (exportDoc)
    classDoc->mDescription = type->Description;

  forRange (CogComponentMeta* metaComponent, type->HasAll<CogComponentMeta>())
    classDoc->mTags.Assign(metaComponent->mTags.All());

  // Loop over all events this meta type sends and save them
  forRange (SendsEvent* eventSent, type->SendsEvents.All())
  {
    classDoc->CreateEventDocFromBoundType(eventSent);
  }

  // get list of all functions (except constructors)
  Array<Function*> allFunctions = type->AllFunctions;

  // add constructors to method list so we get them into the documentation too.
  forRange (Function* constructor, type->Constructors.All())
  {
    allFunctions.Append(constructor);
  }

  // Save all methods
  forRange (Function* method, allFunctions.All())
  {
    if (functionSet.Contains(method))
      continue;

    classDoc->CreateMethodDocFromBoundType(method, replacements, exportDoc);

    // Insert the new method into the map so we can make sure to check for
    // repeats.
    functionSet.Insert(method);
  }

  // Get all properties
  forRange (Property* metaProperty, type->AllProperties.All())
  {
    classDoc->CreatePropertyDocFromBoundType(metaProperty, replacements, exportDoc);
  }
  return classDoc;
}

void DocumentationLibrary::GetDocumentationFromTemplateHandler(StringParam libName, InstantiateTemplateInfo& templateHandler, LibraryBuilder& builder, ArrayMap<String, String>& replacements)
{
  // base name for the template
  String baseName;

  String fullDocTemplateName;

  Array<Constant> dummyTypes;

  // if we have two params, use Wrapper and Void as replaceable dummy
  // types
  if (templateHandler.TemplateParameters.Size() == 2)
  {
    dummyTypes.PushBack(Constant(RaverieTypeId(Wrapper)));
    dummyTypes.PushBack(Constant(RaverieTypeId(Void)));
  }
  else if (templateHandler.TemplateParameters.Size() == 1)
  {
    dummyTypes.PushBack(Constant(RaverieTypeId(Wrapper)));
  }

  String instanceFullName = templateHandler.GetFullName(dummyTypes);
  // currently some types are missing their base name, so create it ourself from
  // the full name
  baseName = GetTemplateBaseName(instanceFullName);

  // save our type replacements so we can remove the dummy types after
  // instantiation
  fullDocTemplateName = InsertIntoReplacementsMap(baseName, instanceFullName, templateHandler, dummyTypes, replacements);

  // create the template instance, possibly instantiating more then one template
  // type
  templateHandler.Delegate.Callback(builder, baseName, fullDocTemplateName, dummyTypes, nullptr);

  // instantiating that template type could have instantiated more then one
  // template get type replacements for them all
  forRange (BoundType* boundTemplateType, builder.BoundTypes.Values().All())
  {
    String newInstanceBaseName = GetTemplateBaseName(boundTemplateType->Name);

    String newInstanceFullName = newInstanceBaseName;

    // if the bound type also has template arguments, add them to the name
    if (!boundTemplateType->TemplateArguments.Empty())
    {
      newInstanceFullName = BuildDocumentationFullTemplateName(newInstanceBaseName, boundTemplateType->TemplateArguments, replacements);
    }

    if (replacements.FindPairPointer(newInstanceBaseName) == nullptr)
    {
      InsertIntoReplacementsMap(newInstanceBaseName, boundTemplateType->Name, templateHandler, dummyTypes, replacements, &newInstanceFullName);
    }
  }

  String name = fullDocTemplateName;

  ClassDoc* classDoc = new ClassDoc();
  classDoc->mName = name;

  classDoc->mLibrary = libName;

  mClasses.PushBack(classDoc);
  mClassMap[classDoc->mName] = classDoc;
  // add version of the class name without template params so it is easier to
  // search for
  mClassMap[GetTemplateBaseName(name)] = classDoc;
}

void DocumentationLibrary::LoadFromMeta()
{
  // First get the range of types from the MetaDatabase
  MetaDatabase* database = MetaDatabase::GetInstance();

  // used to  house the template instances we need to get methods and property
  LibraryBuilder builder("templateLibrary");

  ArrayMap<String, String> allTemplateReplacements;

  forRange (Library* lib, database->mLibraries.All())
  {
    forRange (InstantiateTemplateInfo& templateHandler, lib->TemplateHandlers.Values())
    {
      GetDocumentationFromTemplateHandler(lib->Name, templateHandler, builder, allTemplateReplacements);
    }
  }

  LibraryRef templateLibrary = builder.CreateLibrary();

  forRange (auto& boundTemplate, templateLibrary->BoundTypes.All())
  {
    BoundType* templatedType = boundTemplate.second;

    templatedType->AddAttribute(Raverie::ImportDocumentation);
  }

  // loop over every template type
  forRange (BoundType* metaType, templateLibrary->BoundTypes.Values())
  {
    ClassDoc* newDoc = CreateClassDocFromBoundType(metaType, &allTemplateReplacements);
  }
  // Loop over every nontemplate type
  forRange (BoundType* metaType, database->mTypeMap.Values())
  {
    ClassDoc* newDoc = CreateClassDocFromBoundType(metaType, nullptr);
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
    forRange (PropertyDoc* propertyDoc, classDoc->mProperties.All())
    {
      builder << propertyDoc->mName << " : " << propertyDoc->mType << "\n";
      builder << propertyDoc->mDescription << "\n";
      builder << "\n";
    }
  }

  if (!classDoc->mEventsSent.Empty())
  {
    builder << "Events: \n\n";
    forRange (EventDoc* eventDoc, classDoc->mEventsSent.All())
    {
      builder << "On: " << eventDoc->mName;
      builder << " Sends: " << eventDoc->mType << "\n";
    }
    builder << "\n";
  }

  if (!classDoc->mMethods.Empty())
  {
    builder << "Methods: \n\n";
    forRange (MethodDoc* methodDoc, classDoc->mMethods.All())
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

} // namespace Raverie
