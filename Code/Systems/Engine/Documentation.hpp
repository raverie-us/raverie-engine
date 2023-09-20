// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{
namespace Events
{
DeclareEvent(FillDocumentation);
}

///// FORWARD DECS /////
class MethodDoc;
class DocumentationLibrary;

///// TYPEDEFS /////

/// Replacement map used when loading documentation library from meta
typedef ArrayMap<String, String> TypeReplacementMap;

///// HELPERS /////

/// Find the method doc in a list that has the same parameters as the meta
/// method passed in
MethodDoc* MethodDocWithSameParams(Array<MethodDoc*>& methodList, Raverie::Function* metaMeth);

/// Loads from raverie, saves to data file, then safe deletes
/// Note: Ignoring save unbound for now until feature re-added that used it
void SaveInfoFromMetaToFile(StringParam fileName, bool saveUnbound);

/// Safely deletes the global documentation
void ShutdownDocumentation();

/// Generates a giant string containing nearly all documentation for class by
/// the passed in name
String GenerateDocumentationString(StringParam className);

/// Creates the full template name but with the template type names instead of
/// the instance type names
String BuildDocumentationFullTemplateName(StringParam baseName, Array<Constant>& templateArgs, TypeReplacementMap& replacements);

/// If type is in replacement map (or contains a token that is) return type with
/// that replaced
String ReplaceTypeIfOnList(String& type, TypeReplacementMap* replacements);

/// Insert templated type into the replacement map, used when loading templated
/// types from meta.
void InsertIntoReplacementsMap(InstantiateTemplateInfo& templateHandler, Array<Constant>& dummyTypes, ArrayMap<String, String>& replacements, String* fullName);

///// CLASSES /////
class ExceptionDoc : public Object
{
public:
  void Serialize(Serializer& stream);

  String mTitle;
  String mMessage;
};

/// Contains the name, description, shortcut, and list of tags for a zero
/// command
class CommandDoc : public Object
{
public:
  void Serialize(Serializer& stream);

  /// Name of the command.
  String mName;

  /// Description of what the command will do.
  String mDescription;

  /// Keyboard shortcut.
  String mShortcut;

  /// Tags strings
  Array<String> mTags;
};

/// Wrapper around our command documentation list so we can serialize it
/// properly
class CommandDocList : public Object
{
public:
  void Serialize(Serializer& stream);

  void Sort(void);

  Array<CommandDoc*> mCommands;
};

/// class for documenting all the attributes usable in zero
class AttributeDoc : public Object
{
public:
  AttributeDoc()
  {
  }
  AttributeDoc(AttributeExtension* attribute);
  void Serialize(Serializer& stream);

  String mName;
  String mDescription;
  bool mAllowStatic;
  bool mAllowMultiple;
  bool mDeveloperAttribute;
};

/// Wrapper around our attribute documentation list so we can serialize it
/// properly
class AttributeDocList : public Object
{
public:
  ~AttributeDocList();

  void Serialize(Serializer& stream);

  bool SaveToFile(StringParam fileName);

  void CreateAttributeMap(void);

  void Sort(void);

  HashMap<String, AttributeDoc*> mObjectAttributesMap;
  HashMap<String, AttributeDoc*> mFunctionAttributesMap;
  HashMap<String, AttributeDoc*> mPropertyAttributesMap;

  Array<AttributeDoc*> mObjectAttributes;
  Array<AttributeDoc*> mFunctionAttributes;
  Array<AttributeDoc*> mPropertyAttributes;
};

/// Contains name and type for documented event, usually saved inside of a
/// classDoc
class EventDoc
{
public:
  EventDoc()
  {
  }
  EventDoc(StringParam eventType, StringParam eventName);

  void Serialize(Serializer& stream);

  /// Compares type, and then name if equal, with another event doc so we have a
  /// way of sorting them
  bool operator<(const EventDoc& rhs) const;

  String mName;
  String mType;

  Array<String> mSenders;
  Array<String> mListeners;
};

/// EventDocList actually contains both a map and a list of pointers to event
/// documentation
class EventDocList : public Object
{
public:
  EventDocList()
  {
  }

  void Serialize(Serializer& stream);

  /// Sort list of Event Documentation
  void Sort(void);

  /// Builds the Unsorted map of event docs so events can be found by name
  void BuildMap(void);

  UnsortedMap<String, EventDoc*> mEventMap;

  Array<EventDoc*> mEvents;
};

/// Contains the name, type, and description for a property
class PropertyDoc : public Object
{
public:
  PropertyDoc() : mReadOnly(false), mStatic(false)
  {
  }

  void Serialize(Serializer& stream);

  String mName;

  String mType;

  String mDescription;

  bool mReadOnly;

  bool mStatic;
};

/// Contains the name, type, and description for a param
class ParameterDoc : public Object
{
public:
  void Serialize(Serializer& stream);

  /// Allows comparison of a raverie typeId with a documentation type
  bool IsSameType(DelegateType* type);

  String mType;

  String mDescription;

  String mName;
};

/// Contains the parameters, return type, name, and description for a method
class MethodDoc : public Object
{
public:
  MethodDoc();

  ~MethodDoc();

  void Serialize(Serializer& stream);

  typedef Array<ParameterDoc*> ParameterArray;

  ParameterArray mParameterList;

  Array<ExceptionDoc*> mPossibleExceptionThrows;

  String mParameters;

  String mReturnType;

  String mName;

  String mDescription;

  bool mStatic;
};

/// Contains lists of properties, methods, sent events, and tags as well as the
/// classes name, base class's name, and description for the class
class ClassDoc : public Object
{
public:
  ClassDoc();

  ~ClassDoc();

  void Serialize(Serializer& stream);

  /// Cleans and rebuilds property and method maps as well as sorts arrays
  void BuildMapsAndArrays(void);

  void FillDocumentation(BoundType* classType);

  PropertyDoc* GetPropertyDoc(StringParam propertyName);
  MethodDoc* GetMethodDoc(Raverie::Function* function);

  /// Helper for CreateClassDocFromBoundType to load events
  void CreateEventDocFromBoundType(SendsEvent* eventSent);

  /// Helper for CreateClassDocFromBoundType to load methods
  void CreateMethodDocFromBoundType(Raverie::Function* method, TypeReplacementMap* replacements, bool exportDoc);

  /// Helper for CreateClassDocFromBoundType to load properties
  void CreatePropertyDocFromBoundType(Property* metaProperty, TypeReplacementMap* replacements, bool exportDoc);

  HashMap<String, PropertyDoc*> mPropertiesMap;

  // This is causing the array to be copied when the MethodsMap is changed.
  typedef Array<MethodDoc*> MethodDocArray;
  HashMap<String, MethodDocArray> mMethodsMap;

  UnsortedMap<String, EventDoc*> mEventsMap;

  Array<String> mTags;

  Array<PropertyDoc*> mProperties;

  Array<MethodDoc*> mMethods;

  Array<EventDoc*> mEventsSent;

  String mName;

  String mBaseClass;

  String mDescription;

  String mLibrary;

  ClassDoc* mBaseClassPtr;

  bool mImportDocumentation;

  bool mDevOnly;
};

class EnumDoc : public Object
{
public:
  void Serialize(Serializer& stream);

  void FillDocumentation(BoundType* enumOrFlagType);

  // key is the enumValue and the data is the description
  ArrayMap<String, String> mEnumValues;

  String mName;

  String mDescription;
};

/// Contains list of documented classes, unlike Raw Documentation this is all
/// saved to one file
class DocumentationLibrary : public LazySingleton<DocumentationLibrary, EventObject>
{
public:
  RaverieDeclareType(DocumentationLibrary, TypeCopyMode::ReferenceType);

  ~DocumentationLibrary();

  void Serialize(Serializer& stream);

  void LoadDocumentation(StringParam fileName);

  /// Finalize the documentation for saving/loading (builds maps and fills out
  /// missing data)
  void FinalizeDocumentation();

  /// Helper for CreateClassDocFromBoundType to load enums and flags
  void CreateFlagOrEnumDocFromBoundType(BoundType* type, bool exportDoc);

  /// Helper for LoadFromMeta, checks if a type is in the replacements map
  ClassDoc* CreateClassDocFromBoundType(BoundType* type, TypeReplacementMap* replacements);

  /// Helper for LoadFromMeta, loop for instantiating and loading information
  /// from templated types
  void GetDocumentationFromTemplateHandler(StringParam libName, InstantiateTemplateInfo& templateHandler, LibraryBuilder& builder, ArrayMap<String, String>& allTemplateReplacements);

  /// Load list of classes from the Meta Database
  void LoadFromMeta();

  ClassDoc* GetClassDoc(BoundType* type);

  ClassDoc* GetClassDoc(StringParam className);

  HashMap<String, ClassDoc*> mClassMap;

  HashMap<String, EnumDoc*> mEnumAndFlagMap;

  Array<ClassDoc*> mClasses;

  Array<EnumDoc*> mEnums;

  Array<EnumDoc*> mFlags;
};

namespace Z
{
extern DocumentationLibrary* gDocumentation;
}

} // namespace Raverie
