// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

// Object Attributes
namespace ObjectAttributes
{

/// Object is not visible to default inspection
extern const String cHidden;
/// Object has documentation
extern const String cDocumented;
/// Object should explicitly not be documented
extern const String cDoNotDocument;
/// Object is always expanded in the property grid
extern const String cExpanded;
/// Core objects can not be removed from owner or added to new owner
extern const String cCore;
/// Signifies that this object should store the local property modifications
extern const String cStoreLocalModifications;
/// Object is a Proxy for another class
extern const String cProxy;
/// The proxy was created because we failed to construct an object of this type
/// (likely due to an exception in the constructor)
extern const String cExceptionProxy;
/// For ZilchComponent's, Initialize isn't called in an Editor Space. However,
/// it the Component has this attribute, Initialize will be called.
extern const String cRunInEditor;
/// Used to determine whether a type that 'IsA(Resource)' is an interface, or an
/// actual resource type.
extern const String cResourceInterface;
/// Used for scripts. Allows a component to declare it is an interface for other
/// types (i.e. Collider for BoxCollider)
extern const String cComponentInterface;
/// Used to specify that this component has a gizmo (via the archetype
/// parameter)
extern const String cGizmo;
/// Used to specify that a script component is a Command.
extern const String cCommand;
/// Specify Tags for a script component and its associated Resource.
extern const String cTags;
/// Specify a keyboard shortcut for a script component with the Command
/// attribute.
extern const String cShortcut;
extern const String cTool;

} // namespace ObjectAttributes

// Property Attributes
namespace PropertyAttributes
{

/// Implies both [Display] and [Serialize].
extern const String cProperty;
/// Used to hide members from auto-complete.
extern const String cInternal;
/// When this property is modified in the property grid, the property grid
/// will do a full rebuild allowing for custom property filters to be run
extern const String cInvalidatesObject;
/// When a Graphical is rendered, if this property exists on a Component of the
/// same Cog as the Graphical, it will look for a matching input on the objects
/// material and set the shader parameter.
extern const String cShaderInput;
/// When reverting an object, properties marked with this flag will not be
/// reverted. This is used for things like 'Transform::Translation'. When we
/// revert an object's modifications, we still want it to stay in the same
/// location.
extern const String cLocalModificationOverride;
/// Should this property be serialized. This is implied by
/// PropertyAttributes::cProperty.
extern const String cSerialize;
/// Deprecated. Same functionality as cSerialize. Kept around for legacy files.
extern const String cDeprecatedSerialized;
/// Mark the property as being a dependency on another component.
extern const String cDependency;
/// Should this property show up in the property grid. This is implied by
/// PropertyAttributes::cProperty.
extern const String cDisplay;
/// Deprecated. Same functionality as cDisplay. Kept around for legacy files.
extern const String cDeprecatedEditable;
/// Used for customizing resource properties in scripts.
extern const String cResourceProperty;
/// Clones the Resource
extern const String cRuntimeClone;
/// Used for renaming properties. First attribute parameter should be the old
/// name.
extern const String cRenamedFrom;
/// Indicates the property should be replicated over the network.
extern const String cNetProperty;
/// Inside an Event dispatched over the network, this integer will be
/// automatically filled with the sending NetPeer's NetPeerId.
extern const String cNetPeerId;
/// Used to group properties in the property view. Must have a single string
/// attribute parameter which is the name of the group.
extern const String cGroup;
/// Used to specify a range for numbers being edited in the property grid.
extern const String cRange;
/// Used to specify that the value should be modified by a slider in the
/// property grid.
extern const String cSlider;
/// Used to mark attribute parameters as optional. This is an internal attribute
/// (not exposed to user).
extern const String cOptional;

} // namespace PropertyAttributes

// Function Attributes
namespace FunctionAttributes
{

/// The function shows up in the property grid.
extern const String cProperty;
/// Used to hide the function from auto-complete.
extern const String cInternal;
/// The function shows up in the property grid.
extern const String cDisplay;
/// When this function is called from the property grid, the property grid will
/// do a full rebuild
extern const String cInvalidatesObject;

} // namespace FunctionAttributes

// Serialization Attributes
namespace SerializationAttributes
{

// If it's not a primitive type, serialization will assume it can never be a
// modified property. Only its child properties can be modified. This was used
// to avoid checking if Transform.Translation.X is modified. It can never be
// modified because it's a primitive. Only Transform.Translation can be
// modified.
extern const String cSerializationPrimitive;

} // namespace SerializationAttributes

// Common groups (used with PropertyAttributes::cGroup)
namespace CommonGroups
{
/// A group fpr advanced properties that the user should not touch unless they know what they are doing.
extern const String cAdvanced;
} // namespace CommonGroups

#define ZeroAdvancedGroup()                                                                                            \
  AddAttribute(::Zero::PropertyAttributes::cGroup)->AddParameter(::Zero::CommonGroups::cAdvanced)

// Uncategorized
extern const String cInvalidTypeName;

#define ZeroBindDocumented() type->AddAttribute(::Zero::ObjectAttributes::cDocumented)
#define ZeroBindExpanded() type->AddAttribute(::Zero::ObjectAttributes::cExpanded)

#define ZeroBindSetup(SetupMode) type->HasOrAdd<::Zero::CogComponentMeta>(type)->mSetupMode = (SetupMode)
#define ZeroBindDependency(Type) type->HasOrAdd<::Zero::CogComponentMeta>(type)->mDependencies.Insert(ZilchTypeId(Type))
#define ZeroBindInterface(Type) type->HasOrAdd<::Zero::CogComponentMeta>(type)->AddInterface(ZilchTypeId(Type))
#define ZeroBindTag(Tag) type->HasOrAdd<::Zero::CogComponentMeta>(type)->mTags.Insert(Tag)
#define ZeroBindPropertyRename(oldName) Add(new ::Zero::MetaPropertyRename(oldName))
#define ZeroSetPropertyGroup(groupName) Add(new ::Zero::MetaGroup(groupName))
#define ZeroLocalModificationOverride() AddAttribute(::Zero::PropertyAttributes::cLocalModificationOverride)

void BindEventSent(LibraryBuilder& builder, BoundType* boundType, StringParam eventName, BoundType* eventType);
#define ZeroBindEvent(EventName, EventType) BindEventSent(builder, type, (EventName), ZilchTypeId(EventType))
#define ZeroBindExternalEvent(EventName, EventType, SenderType)                                                        \
  BindEventSent(builder, ZilchTypeId(SenderType), (EventName), ZilchTypeId(EventType))

// Events
namespace Events
{
DeclareEvent(PropertyModified);
DeclareEvent(PropertyModifiedIntermediate);

// The objects Components were changed
DeclareEvent(ComponentsModified);

// Used to signal to the property grid that it should rebuild. Will be sent when
// Components are modified
DeclareEvent(ObjectStructureModified);

// The object was changed in any way (will be sent when PropertyChanged,
// ComponentsChanged, and ObjectStructureModified are sent)
DeclareEvent(ObjectModified);
} // namespace Events

// Meta Operations
// If the object supports undo/redo, this component should be added
class MetaOperations : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(MetaOperations, TypeCopyMode::ReferenceType);

  // When a property is changed in the editor, this should be called to properly
  // send events or run any special functionality per object type.
  static void NotifyPropertyModified(
      HandleParam object, PropertyPathParam property, AnyParam oldValue, AnyParam newValue, bool intermediateChange);
  static void NotifyComponentsModified(HandleParam object);
  // Called when an object is modified in any way.
  static void NotifyObjectModified(HandleParam object, bool intermediateChange = false);

  // Id used in the UndoMap for the operation system. Currently, this id needs
  // to be globally unique between everything that implements this function.
  // This could be made better by having an undo map per type, instead of a
  // global one.
  virtual u64 GetUndoHandleId(HandleParam object);

  // Before all meta operations, this will be called and stored. When Undo is
  // called, RestoreUndoData will be called with the previously stored data.
  virtual Any GetUndoData(HandleParam object);

  // The given property was modified. This will be called on parents when a
  // child property is modified, as well as the child.
  //
  // e.g. When Cog.CameraViewport.CameraPath.Path is changed, you will get the
  // events on
  // 1. Cog instance with "CameraViewport/CameraPath/Path" as the PropertyPath
  // 2. CameraViewport instance with "CameraPath/Path" as the PropertyPath
  // 3. Path instance with "Path" as the PropertyPath
  virtual void PropertyModified(
      HandleParam object, PropertyPathParam property, AnyParam oldValue, AnyParam newValue, bool intermediateChange);

  // Called when any Component is added / moved / removed.
  virtual void ComponentsModified(HandleParam object);

  // The object was modified in any way (property modified, component
  // added/moved/removed, property marked as modified, property reverted, child
  // restored, child order restored, etc...)
  virtual void ObjectModified(HandleParam object, bool intermediateChange);

  // When an operation is undone, the data returned from 'GetUndoData' will be
  // given back to us here.
  virtual void RestoreUndoData(HandleParam object, AnyParam undoData)
  {
  }

  // Some editor operations require storing the state of and rebuilding of
  // objects. Look at ObjectRestoreState for more detail.
  virtual ObjectRestoreState* GetRestoreState(HandleParam object)
  {
    return nullptr;
  }
};

// Property Event
class PropertyEvent : public Event
{
public:
  ZilchDeclareType(PropertyEvent, TypeCopyMode::ReferenceType);

  PropertyEvent(HandleParam object, PropertyPathParam property, AnyParam oldValue, AnyParam newValue);

  Handle mObject;
  const PropertyPath& mProperty;
  Any mOldValue;
  Any mNewValue;
};

// Type Event
class TypeEvent : public Event
{
public:
  ZilchDeclareType(TypeEvent, TypeCopyMode::ReferenceType);
  TypeEvent(BoundType* type) : mType(type)
  {
  }

  BoundType* mType;
};

// Meta Display
class MetaDisplay : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(MetaDisplay, TypeCopyMode::ReferenceType);

  virtual String GetName(HandleParam object) = 0;
  virtual String GetDebugText(HandleParam object) = 0;
};

// Type Name Display
class TypeNameDisplay : public MetaDisplay
{
public:
  ZilchDeclareType(TypeNameDisplay, TypeCopyMode::ReferenceType);

  String GetName(HandleParam object) override;
  String GetDebugText(HandleParam object) override;
};

// String Name Display
class StringNameDisplay : public MetaDisplay
{
public:
  ZilchDeclareType(StringNameDisplay, TypeCopyMode::ReferenceType);
  StringNameDisplay(StringParam string);

  String GetName(HandleParam object) override;
  String GetDebugText(HandleParam object) override;

  String mString;
};

// Meta Transform Instance
class MetaTransformInstance
{
public:
  MetaTransformInstance(HandleParam instance = Handle()) :
      mInstance(instance),
      mLocalTranslation(nullptr),
      mLocalRotation(nullptr),
      mLocalScale(nullptr),
      mWorldTranslation(nullptr),
      mWorldRotation(nullptr),
      mWorldScale(nullptr),
      mParentWorldMatrix(nullptr),
      mParentLocalMatrix(nullptr)
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
  // Local Matrix
  Mat4 GetParentLocalMatrix();

  // Transform a local space point to parent space.
  Vec3 ToParent(Vec3Param local);

  // Transform for Cog, object instance for GeoElement, etc...
  Handle mInstance;

  // Consider removing this. It was added to mark the space as modified in
  // Gizmos, but that should be handle in the operation queue.
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
  Property* mParentLocalMatrix;

  // Used to get a size for focusing. Potentially remove and refactor later.
  Aabb mAabb;
};

typedef MetaTransformInstance& MetaTransformParam;

// Meta Transform
// Allows objects to have transform type properties without the limitation of
// the object type being a Cog
class MetaTransform : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(MetaTransform, TypeCopyMode::ReferenceType);
  virtual MetaTransformInstance GetInstance(HandleParam object) = 0;
};

// The "has" macro
#define has(type) Has<type>()

// An extension to the "has" macro
#define hasAll(type) HasRange<type>()

// Array Binding
#define ZeroDefineArrayType(arrayType)                                                                                 \
  ZilchDefineTemplateType(ZeroMetaArray<arrayType>, builder, type)                                                     \
  {                                                                                                                    \
  }                                                                                                                    \
                                                                                                                       \
  ZilchDefineExternalBaseType(arrayType, TypeCopyMode::ReferenceType, builder, type)                                   \
  {                                                                                                                    \
    type->HandleManager = ZilchManagerId(PointerManager);                                                              \
    type->Add(new ZeroMetaArray<arrayType>());                                                                         \
  }

#define ZeroInitializeArrayTypeAs(arrayType, name)                                                                     \
  ZilchInitializeTypeAs(ZeroMetaArray<arrayType>, "ZeroMetaArray" name);                                               \
  ZilchInitializeExternalTypeAs(arrayType, name);

// Meta Attribute
class MetaAttribute : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(MetaAttribute, TypeCopyMode::ReferenceType);
  virtual void PostProcess(Status& status, ReflectionObject* owner)
  {
  }
};

} // namespace Zero
