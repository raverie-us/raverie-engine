///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Joshua Claeys
/// Copyright 2016-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------------------- Object Attributes
namespace ObjectAttributes
{

// Object is not visible to default inspection
extern const String cHidden;
// Object has documentation
extern const String cDocumented;
// Object is always expanded in the property grid
extern const String cExpanded;
// Core objects can not be removed from owner or added to new owner
extern const String cCore;
// Signifies that this object should store the local property modifications
extern const String cStoreLocalModifications;
// Object is a Proxy for another class
extern const String cProxy;
// The proxy was created because we failed to construct an object of this type (likely due to 
// an exception in the constructor)
extern const String cExceptionProxy;
// For ZilchComponent's, Initialize isn't called in an Editor Space. However, it the Component has
// this attribute, Initialize will be called.
extern const String cRunInEditor;
// Used to determine whether a type that 'IsA(Resource)' is an interface, or an actual resource type.
extern const String cResourceInterface;
// Used for scripts. Allows a component to declare it is an interface for other types (i.e. Collider for BoxCollider)
extern const String cComponentInterface;
// Used to specify that this component has a gizmo (via the archetype parameter)
extern const String cGizmo;

}//namespace ObjectFlags

 //------------------------------------------------------------------------------ Property Attributes
namespace PropertyAttributes
{

// The property cannot be modified by the user
extern const String cReadOnly;
// Only shows up in the property grid if the user has DevConfig
extern const String cCore;
// When this property is modified in the property grid, the property grid
// will do a full rebuild allowing for custom property filters to be run
extern const String cInvalidatesObject;
// When a Graphical is rendered, if this property exists on a Component of the same Cog 
// as the Graphical, it will look for a matching input on the objects material 
// and set the shader parameter.
extern const String cShaderInput;
// When reverting an object, properties marked with this flag will not be reverted. This is
// used for things like 'Transform::Translation'. When we revert an object's modifications,
// we still want it to stay in the same location.
extern const String cLocalModificationOverride;
// When this is bound as a property, ignore its child properties and save in a custom way
// Can each of my sub properties be independently locally modified? This was added for
// saving out local modifications to CogPath.
extern const String cAsPropertyUseCustomSerialization;
// Should this property be serialized. This is implied by Zilch::PropertyAttribute,
// but this is also added by ZeroSerialize.
extern const String cSerialized;
// Mark the property as being a dependency on another component.
extern const String cDependency;
// Should this property show up in the property grid.
extern const String cEditable;
// Used for customizing resource properties in scripts.
extern const String cResourceProperty;
// Used for renaming properties.
extern const String cRenamedFrom;

}//namespace PropertyFlags

//------------------------------------------------------------------------------ Function Attributes
namespace FunctionAttributes
{

// The property cannot be modified by the user
extern const String cProperty;

// When this function is called from the property grid, the property grid will do a full rebuild
extern const String cInvalidatesObject;

}//namespace FunctionFlags

 // Uncategorized
extern const String cInvalidTypeName;

#define ZeroBindDocumented() type->AddAttribute(::Zero::ObjectAttributes::cDocumented)
#define ZeroBindExpanded() type->AddAttribute(::Zero::ObjectAttributes::cExpanded)

#define ZeroBindSetup(SetupMode) type->HasOrAdd<::Zero::CogComponentMeta>(type)->mSetupMode = (SetupMode)
#define ZeroBindDependency(Type) type->HasOrAdd<::Zero::CogComponentMeta>(type)->mDependencies.Insert(ZilchTypeId(Type))
#define ZeroBindInterface(Type)  type->HasOrAdd<::Zero::CogComponentMeta>(type)->AddInterface(ZilchTypeId(Type))
#define ZeroBindTag(Tag)         type->HasOrAdd<::Zero::CogComponentMeta>(type)->mTags.Insert(Tag)
#define ZeroBindPropertyRename(oldName)  Add(new ::Zero::MetaPropertyRename(oldName))

void BindEventSent(LibraryBuilder& builder, BoundType* boundType, StringParam eventName, BoundType* eventType);
#define ZeroBindEvent(EventName, EventType)  BindEventSent(builder, type, (EventName), ZilchTypeId(EventType))

// Used for adding custom Ui to the property grid
class MetaCustomUi : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  virtual void CreateUi(void* parentComposite, HandleParam object) = 0;
};

//------------------------------------------------------------------------------------------- Events
namespace Events
{
DeclareEvent(PropertyModified);
DeclareEvent(PropertyModifiedIntermediate);

// The objects Components were changed
DeclareEvent(ComponentsModified);

// Used to signal to the property grid that it should rebuild. Will be sent when Components are modified
DeclareEvent(ObjectStructureModified);

// The object was changed in any way (will be sent when PropertyChanged, ComponentsChanged, and
// ObjectStructureModified are sent)
DeclareEvent(ObjectModified);
}

//---------------------------------------------------------------------------------- Meta Operations
// If the object supports undo/redo, this component should be added
class MetaOperations : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // When a property is changed in the editor, this should be called to properly send events or
  // run any special functionality per object type.
  static void NotifyPropertyModified(HandleParam object, PropertyPathParam property,
                                    AnyParam oldValue, AnyParam newValue, bool intermediateChange);
  static void NotifyComponentsModified(HandleParam object);
  static void NotifyObjectModified(HandleParam object);

  // Id used in the UndoMap for the operation system. Currently, this id needs to be globally
  // unique between everything that implements this function. This could be made better by 
  // having an undo map per type, instead of a global one.
  virtual u64 GetUndoHandleId(HandleParam object);

  // Before all meta operations, this will be called and stored. When Undo is called,
  // RestoreUndoData will be called with the previously stored data.
  virtual Any GetUndoData(HandleParam object);

  // The given property was modified. This will be called on parents when a child property
  // is modified, as well as the child.
  //
  // e.g. When Cog.CameraViewport.CameraPath.Path is changed, you will get the events on
  // 1. Cog instance with "CameraViewport/CameraPath/Path" as the PropertyPath
  // 2. CameraViewport instance with "CameraPath/Path" as the PropertyPath
  // 3. Path instance with "Path" as the PropertyPath
  virtual void PropertyModified(HandleParam object, PropertyPathParam property, AnyParam oldValue,
                                AnyParam newValue, bool intermediateChange);

  // Called when any Component is added / moved / removed.
  virtual void ComponentsModified(HandleParam object);

  // The object was modified in any way (property modified, component added/moved/removed, property
  // marked as modified, property reverted, child restored, child order restored, etc...)
  virtual void ObjectModified(HandleParam object);

  // When an operation is undone, the data returned from 'GetUndoData' will be given back to us here.
  virtual void RestoreUndoData(HandleParam object, AnyParam undoData) { }

  // Some editor operations require storing the state of and rebuilding of objects. Look at
  // ObjectRestoreState for more detail.
  virtual ObjectRestoreState* GetRestoreState(HandleParam object) { return nullptr; }
};

//----------------------------------------------------------------------------------- Property Event
class PropertyEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PropertyEvent(HandleParam object, PropertyPathParam property,
                AnyParam oldValue, AnyParam newValue);

  Handle mObject;
  const PropertyPath& mProperty;
  Any mOldValue;
  Any mNewValue;
};

//--------------------------------------------------------------------------------------- Type Event
class TypeEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  TypeEvent(BoundType* type) : mType(type) {}

  BoundType* mType;
};

class TemplateFilterBase
{
public:
  virtual bool Filter(Property* prop, HandleParam instance) = 0;
};

template <typename ClassType, bool ClassType::* Member>
class TemplateFilterBool : public TemplateFilterBase
{
public:
  bool Filter(Property* prop, HandleParam instance) override
  {
    ClassType* pointer = instance.Get<ClassType*>(GetOptions::AssertOnNull);
    return pointer->*Member;
  }
};

template <typename ClassType, bool ClassType::* Member>
class TemplateFilterNotBool : public TemplateFilterBase
{
public:
  bool Filter(Property* prop, HandleParam instance) override
  {
    ClassType* pointer = instance.Get<ClassType*>(GetOptions::AssertOnNull);
    return !(pointer->*Member);
  }
};

template <typename ClassType, typename ValueType, ValueType ClassType::* Member, ValueType Value>
class TemplateFilterEquality : public TemplateFilterBase
{
public:
  bool Filter(Property* prop, HandleParam instance) override
  {
    ClassType* pointer = instance.Get<ClassType*>(GetOptions::AssertOnNull);
    return pointer->*Member == Value;
  }
};

class MetaPropertyFilter : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  virtual ~MetaPropertyFilter() {}

  // Return false to hide the property
  virtual bool Filter(Property* prop, HandleParam instance) = 0;
};

class MetaPropertyBasicFilter : public MetaPropertyFilter
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MetaPropertyBasicFilter(TemplateFilterBase* filter = nullptr) : mActualFilter(filter) {}
  ~MetaPropertyBasicFilter() { if (mActualFilter) delete mActualFilter; }

  bool Filter(Property* prop, HandleParam instance)
  {
    return mActualFilter->Filter(prop, instance);
  }

  TemplateFilterBase* mActualFilter;
};

class MetaEditorGizmo : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  String mGizmoArchetype;
};

class MetaDisplay : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  virtual String GetName(HandleParam object) = 0;
  virtual String GetDebugText(HandleParam object) = 0;
};

class TypeNameDisplay : public MetaDisplay
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  String GetName(HandleParam object) override;
  String GetDebugText(HandleParam object) override;
};

class StringNameDisplay : public MetaDisplay
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  StringNameDisplay(StringParam string);

  String GetName(HandleParam object) override;
  String GetDebugText(HandleParam object) override;

  String mString;
};

#define ZeroFilterBool(Member)                                      \
  Add(new MetaPropertyBasicFilter(new TemplateFilterBool<ZilchSelf, &ZilchSelf::Member>()))

#define ZeroFilterNotBool(Member)                                   \
  Add(new MetaPropertyBasicFilter(new TemplateFilterNotBool<ZilchSelf, &ZilchSelf::Member>()))

#define ZeroFilterEquality(Member, MemberType, ConstantValue)       \
  Add(new MetaPropertyBasicFilter(new TemplateFilterEquality<ZilchSelf, MemberType, &ZilchSelf::Member, ConstantValue>()))

//----------------------------------------------------------------------------------- Meta Transform
class MetaTransformInstance
{
public:
  MetaTransformInstance(HandleParam instance = Handle())
    : mInstance(instance),
    mLocalTranslation(nullptr),
    mLocalRotation(nullptr),
    mLocalScale(nullptr),
    mWorldTranslation(nullptr),
    mWorldRotation(nullptr),
    mWorldScale(nullptr),
    mParentWorldMatrix(nullptr)
  {
    // Initialize the aabb to an invalid one so that we can take any other aabb
    // and combine with this without anything being affected.
    mAabb.SetInvalid();
  }

  // Returns whether or not the instance is null.
  bool IsNull();
  bool IsNotNull();

  // Local Transform
  Vec3 GetLocalTranslation();
  Quat GetLocalRotation();
  Vec3 GetLocalScale();

  void SetLocalTranslation(Vec3Param value);
  void SetLocalRotation(QuatParam value);
  void SetLocalScale(Vec3Param value);

  // World Transform
  Vec3 GetWorldTranslation();
  Quat GetWorldRotation();
  Vec3 GetWorldScale();

  // World Matrix
  Mat4 GetParentWorldMatrix();

  // Transform for cog, object instance for geoelement, etc...
  Handle mInstance;

  // Consider removing this. It was added to mark the space as modified in Gizmos, but
  // that should be handle in the operation queue.
  Handle mSpace;

  // Local Transform Properties
  Property* mLocalTranslation;
  Property* mLocalRotation;
  Property* mLocalScale;

  // World Transform Properties
  Property* mWorldTranslation;
  Property* mWorldRotation;
  Property* mWorldScale;

  // Used to get the parents world matrix (your local space)
  Handle mParentInstance;
  Property* mParentWorldMatrix;

  // Used to get a size for focusing. Potentially remove and refactor later.
  Aabb mAabb;
};

// Allows objects to have transform type properties without the limitation of
// the object type being a Cog
class MetaTransform : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  virtual MetaTransformInstance GetInstance(HandleParam object) = 0;
};

//The "has" macro
#define has(type) Has<type>()

//An extension to the "has" macro
#define hasAll(type) HasRange<type>()

//------------------------------------------------------------------------------------ Array Binding
#define ZeroDefineArrayType(arrayType)                                                \
  ZilchDefineType(ZeroMetaArray<arrayType>, builder, type)                            \
  {                                                                                   \
                                                                                      \
  }                                                                                   \
                                                                                      \
  ZilchDefineExternalBaseType(arrayType, TypeCopyMode::ReferenceType, builder, type)  \
  {                                                                                   \
    type->HandleManager = ZilchManagerId(PointerManager);                             \
    type->Add(new ZeroMetaArray<arrayType>());                                        \
  }

#define ZeroInitializeArrayTypeAs(arrayType, name)  \
  ZilchInitializeType(ZeroMetaArray<arrayType>);    \
  ZilchInitializeExternalTypeAs(arrayType, name);

//---------------------------------------------------------------------------------- Property Rename
/// Add to properties to handle old files with old property names.
class MetaPropertyRename : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  
  MetaPropertyRename(StringParam oldName);

  String mOldName;
};

} // namespace Zero
