////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------------------------------------------- Events
namespace Events
{
  DeclareEvent(CogNameChanged);
  DeclareEvent(TransformUpdated);
  DeclareEvent(CogDestroy);
  DeclareEvent(CogDelayedDestroy);
}

class Archetype;
struct TransformUpdateInfo;

// Type define for a range
typedef ConditionalRange<HierarchyRange, NameCondition> HierarchyNameRange;

DeclareBitField10(CogFlags,
                  // Object cannot be destroyed be the user
                  Protected,
                  // Object will not be destroy on level change or clear
                  Persistent,
                  // Object is not saved in a level (Temporary objects from particles systems etc)
                  Transient,
                  // Object has been initialized
                  Initialized,
                  // Object has been destroyed
                  Destroyed,
                  // Object was loaded from a level file
                  CreatedFromLevel,
                  // Hidden from view used for editor
                  EditorViewportHidden,
                  // If the object needs to not show up in the object view
                  ObjectViewHidden,
                  // Not able to be modified or selected in the viewport
                  Locked,
                  // Can only be selected by SelectionIcon
                  SelectionLimited);

//----------------------------------------------------------------------------------------- Base Cog
/// Base class used for the intrusive link.
class BaseCog : public Object
{
public:
  IntrusiveLink(BaseCog, HierarchyLink);
};

typedef BaseInList<BaseCog, Cog, &BaseCog::HierarchyLink> HierarchyList;

//---------------------------------------------------------------------------------------------- Cog
/// Game Object Composition class. This class is the foundational object for all
/// dynamic objects in the game world. The Cog is a piece of logical interactive 
/// content and the primary mechanism game systems (Graphics, Physics, Etc.) 
/// provide functionality and communicate. A Cog can be anything from physical 
/// objects like trees, tanks, players to logical objects like teams, 
/// triggers, or AI objects. 
class Cog : public BaseCog
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Memory Allocation  
  OverloadedNew();
  static Memory::Heap* sHeap;

  /// Constructor / destructor.
  Cog();
  virtual ~Cog();

  /// Queues the cog up for delayed destruction (at the end of the frame).
  /// If the object is marked as Protected, this will do nothing.
  virtual void Destroy();
  /// Queues the cog up for delayed destruction (at the end of the frame).
  /// Ignores the Protected flag.
  virtual void ForceDestroy();
  /// Called when the Cog is destroyed. Allows for custom destruction behavior.
  virtual void OnDestroy();

  /// Name of the Object.
  String GetName( );
  virtual void SetName(StringParam newName);

  /// Returns the Space that this object lives in.
  virtual Space* GetSpace();
  /// Get the GameSession that owns us and our Space.
  virtual GameSession* GetGameSession();
  /// Get the object named 'LevelSettings', a special convenience object where we can put general
  /// functionality for our Level.
  Cog* GetLevelSettings();

  /// Clones this cog. The cloned object will be parented to this objects parent (if it exists).
  virtual Cog* Clone();

  //----------------------------------------------------------------------------- Serialization
  /// Serialize all components data.
  void Serialize(Serializer& stream) override;

  //----- Internals
  static DataBlock SaveToDataBlock(Cog* cog);
  static Cog* CreateFromDataBlock(Space* space, DataBlock& block);
  static Cog* CreateFromString(Space* space, StringParam stringData);
  static Cog* CreateAndInitializeFromStream(Space* space, Serializer& stream);

  //---------------------------------------------------------------------------- Initialization
  /// Initialize all the components on the composition.
  /// The order of initialize is as follows:
  /// 1. Initialize
  /// 2. OnAllObjectsCreated
  /// 3. ScriptInitialize
  /// 4. The event 'AllObjectsInitialized' is sent out on the CogInitializer
  /// Generally script Initialize comes after ALL C++ initialization because
  /// it makes scripts safer and easier to debug
  /// (knowing all C++ components have been fully setup, can't access anything bad)
  virtual void Initialize(CogInitializer& initializer);
  virtual void OnAllObjectsCreated(CogInitializer& initializer);
  virtual void ScriptInitialize(CogInitializer& initializer);
  /// Returns whether or not the object has already been initialized.
  bool IsInitialized() const;

  //-------------------------------------------------------------------------------- Components
  /// Typedefs.
  typedef Array<Component*> ComponentArray;
  typedef ComponentArray::range ComponentRange;
  typedef ArrayMultiMap<BoundType*, Component*> ComponentMap;
  typedef ComponentMap::valueRange ComponentMapRange;

  //----- Component Addition
  /// Add a component of the given type.
  bool AddComponentByType(BoundType* componentType);
  /// Add a component by name.
  bool AddComponentByName(StringParam name);
  /// Adds the given Component at the given index. If the index is -1, it will be placed at the end.
  /// Returns false if the Component couldn't be added due to missing dependencies or if there
  /// was already a Component of the same type.
  bool AddComponent(Component* component, int index = -1);
  /// Adds the Component without checking for dependencies. If the index is -1, it will be placed
  /// at the end. This was added mostly as an optimization where we know dependency
  /// checking is not required.
  void ForceAddComponent(Component* component, int index = -1);

  //----- Component Querying
  /// Type safe way of accessing components.
  template<typename type>
  type* Has();
  /// Finds a component of the given type.
  Component* QueryComponentType(BoundType* componentType);
  /// Finds the Components with the given type name.
  Component* GetComponentByName(StringParam componentTypeName);
  /// Returns the Component at the given index.
  Component* GetComponentByIndex(size_t index);
  /// Returns how many Components are on this Cog.
  size_t GetComponentCount();
  /// Finds the index of the given Component type. Returns uint max if the Component didn't exist.
  uint GetComponentIndex(BoundType* componentType);
  /// Returns a range of all Components on this Cog.
  ComponentRange GetComponents();

  //----- Component Removal
  /// Remove a component by type.  Returns true if the component existed.
  bool RemoveComponentByType(BoundType* componentType);
  /// Remove a component by name.  Returns true if the component existed.
  bool RemoveComponentByName(StringParam typeName);
  /// Removes the given Component from the Cog. Returns false if the Component couldn't be removed
  /// due to dependencies (cannot remove Transform if it has a Model).
  bool RemoveComponent(Component* component);
  /// Removes the given Component without checking dependencies. This was added mostly as
  /// an optimization
  void ForceRemoveComponent(Component* component);
  /// Moves the component from one index to another.
  /// This function assumes that the move is valid (meaning it's not moving
  /// a component before another component that it's dependent on).
  void MoveComponentBefore(uint componentToMove, uint destination);

  //----- Internals
  /// Add a component interface. This was added mostly as an optimization where we know dependency
  /// checking is not required.
  void AddComponentInterface(BoundType* alternateType, Component* component);
  /// Add component to internal map used mostly during construction
  void AddComponentInternal(BoundType* typeId, Component* component, int index = -1);
  /// Removes the component from the internal map and deletes the component.
  void RemoveComponentInternal(Component* component);
  /// Checks for 
  bool CheckForAddition(BoundType* componentType);
  /// Helper to check for dependencies with component addition and DoNotify if it won't
  bool CheckForAdditionWithNotify(BoundType* componentType);
  /// Helper to check for dependencies with component removal and DoNotify if it won't
  bool CheckForRemovalWithNotify(BoundType* componentType);
  /// Private, use Destroy function to destroy compositions.
  void DeleteComponents();

  /// Ordered list of Components. Components will be initialized in the order of this Array.
  ComponentArray mComponents;
  /// The map of the component's name to their instance for fast lookup.
  ComponentMap mComponentMap;

  //---------------------------------------------------------------------------------- Children
  /// Get the parent of this object in the Hierarchy.
  Cog* GetParent();
  /// Searches up the hierarchy for the root Cog.
  Cog* FindRoot();
  /// Returns a range of all direct children on this Cog.
  HierarchyList::range GetChildren();
  /// Returns the amount of children on this Cog.
  uint GetChildCount();

  /// Attach to a parent object.
  void AttachTo(Cog* parent);
  /// Attach to a parent object and compute the new transform so that the objects are relative
  void AttachToRelative(Cog* parent);
  /// Detach from a parent object.
  void Detach();
  /// Detach from a parent object and compute the new transform so that the objects are relative
  void DetachRelative();

  /// Find a child object with the given name
  Cog* FindChildByName(StringParam name);
  /// Returns a range of all children with the given name.
  HierarchyNameRange FindAllChildrenByName(StringParam name);
  /// Returns the child object with the given id. This is only checks direct children.
  Cog* FindChildByChildId(Guid childId);

  /// Returns whether or not the given cog is a descendant of us.
  bool IsDescendant(Cog* cog);

  /// Returns the sibling Cog after this in the parents child list. Returns null if it's the 
  /// last child. If the Cog doesn't have a parent, it will return the Cog after it in the Space.
  Cog* FindNextSibling();
  /// Returns the sibling Cog before this in the parents child list. Returns null if it's the 
  /// first child. If the Cog doesn't have a parent, it will return the Cog before it in the Space.
  Cog* FindPreviousSibling();
  /// Finds the next Cog in depth first post-order.
  Cog* FindNextInOrder();
  /// Finds the previous Cog in reverse depth first post-order (the opposite of FindNextInOrder).
  Cog* FindPreviousInOrder();

  /// Moves this Cog before the given sibling. Assumes they have the same parent.
  void PlaceBeforeSibling(Cog* sibling);
  /// Moves this Cog after the given sibling. Assumes they have the same parent.
  void PlaceAfterSibling(Cog* sibling);
  /// Places the new child at the same place as the old child in the Hierarchy.
  /// This detaches but does not destroy the old child.
  void ReplaceChild(Cog* oldChild, Cog* newChild);

  /// Returns this Cogs index in the parents children list. If it doesn't have a parent, it will
  /// return the index in the Space's object list.
  uint GetHierarchyIndex();

  //----- Internals
  /// Moves the object to the given index in the parents child list. This currently does no
  /// bounds checking as an optimization.
  void PlaceInHierarchy(uint index);
  Cog* FindLastDeepestChild();
  /// Returns the list of all children.
  HierarchyList* GetHierarchyList();
  /// Returns the list of all of our parents children. If we don't have a parent, it will return
  /// the list of all objects in the Space.
  HierarchyList* GetParentHierarchyList();

  /// The parent of this Cog in a Hierarchy.
  Cog* mHierarchyParent;

  //-------------------------------------------------------------------------------- Archetypes
  /// Getter / setter for Archetype.
  Archetype* GetArchetype();
  void SetArchetype(Archetype* archetype);
  /// Returns the Archetype our Archetype inherits from.
  Archetype* GetBaseArchetype();
  String GetBaseArchetypeName();

  /// Returns whether or not we have any local modifications from our Archetype. This does not
  /// account for properties with LocalModificationOverride (such as Transform modifications).
  bool IsModifiedFromArchetype();
  /// Removes our association with the current Archetype.
  void ClearArchetype();
  /// Clears all modifications on this Cog. Does not clear LocalModificationOverride properties.
  void MarkNotModified();
  /// Upload this objects data to the archetype and marks this object as not modified.
  /// This function does not rebuild all other Cogs with the same Archetype. See ArchetypeRebuilder
  /// for more information about how to rebuild Archetypes.
  void UploadToArchetype();
  /// Removes all local modifications and rebuilds the Cog.
  void RevertToArchetype();
  /// Marks all Transform properties as modified. This is a common operation when creating a
  /// Cog from Archetype in the editor. We want the Transform modified when we create it in the
  /// scene so that it's location is saved when the level is saved.
  void MarkTransformModified();

  /// This will find either the root Archetype, or the nearest locally added Archetype.
  Cog* FindNearestArchetypeContext();
  /// Finds the nearest parent that has an Archetype (does not include itself).
  Cog* FindNearestParentArchetype();
  /// Same as FindNearestParentArchetype except that it includes this Cog.
  Cog* FindNearestArchetype();
  /// Finds the top most Archetype in the Hierarchy.
  Cog* FindRootArchetype();
  /// Whether or not this object is a child of an Archetype (not only direct
  /// child, but any child).
  bool IsChildOfArchetype();
  
  //----- Internals
  /// Archetype this object was created with.
  HandleOf<Archetype> mArchetype;

  //------------------------------------------------------------------------------------ Events
  /// Dispatches an event on this object
  void DispatchEvent(StringParam eventId, Event* event);

  /// Dispatches an event up the tree on each parent recursively (pre-order traversal)
  void DispatchUp(StringParam eventId, Event* event);

  /// Dispatches an event down the tree on all children recursively (pre-order traversal)
  void DispatchDown(StringParam eventId, Event* event);

  //----- Internals
  EventDispatcher* GetDispatcherObject();
  EventReceiver* GetReceiverObject();
  EventDispatcher* GetDispatcher();
  EventReceiver* GetReceiver();

  EventReceiver mReceiver;
  EventDispatcher mDispatcher;

  //------------------------------------------------------------------------------------- Other
  virtual bool IsEditorMode();
  virtual bool IsPreviewMode();
  virtual bool IsEditorOrPreviewMode();

  /// Calls DebugDraw on all components in this cog.
  virtual void DebugDraw();

  /// Cleans an object name of invalid runes.
  static String SanatizeName(StringParam newName);

  ///Get the game object's Id
  CogId GetId();
  ///Gets a unique integer for this object (used primarily for debugging)
  u32 GetRuntimeId();

  /// Sets Protected and Transient.
  void SetEditorOnly();
  /// Object will not be saved.
  bool GetTransient();
  void SetTransient(bool transient);
  /// Object will not be destroyed on level load or change.
  bool GetPersistent();
  void SetPersistent(bool persistent);
  /// Hidden from view used for editor.
  bool GetEditorViewportHidden();
  void SetEditorViewportHidden(bool state);
  /// If the object needs to not show up in the object view.
  bool GetObjectViewHidden();
  void SetObjectViewHidden(bool state);
  /// Not able to be modified or selected in the viewport.
  bool GetLocked();
  void SetLocked(bool state);

  /// When the object is moved, this should be called to inform all Components that it has moved.
  /// It also sends an event.
  void TransformUpdate(TransformUpdateInfo& info);

  /// Has this Cog already been destroyed and is waiting for the frame to end (delayed destruction).
  /// This allows us to do custom logic when an object is still not null, but about to be destroyed
  /// (e.g. we don't want to render Cogs marked for deletion).
  bool GetMarkedForDestruction() const;

  void WriteDescription(StringBuilder& builder);
  String GetDescription();
  Actions* GetActions();

  /// The name of this object.
  String mName;
  /// Space this composition is in.
  Space* mSpace;
  /// A unique id for each object used to safely reference Cogs.
  CogId mObjectId;
  /// A unique id (unique within its siblings) that's used to map local changes
  /// made to Archetype child objects.
  Guid mChildId;
  /// Composition Flags.
  BitField<CogFlags::Enum> mFlags;
  /// This is the sub context that the cog was loaded into. This is used to
  /// properly restore CogId's on CogPath. See the ContextId's section in the
  /// source documentation for a more in depth explanation.
  uint mSubContextId;

  /// Action list.
  Actions* mActionList;
  ///Link for all game objects in space.
  Link<Cog> SpaceLink;
  Link<Cog> NameLink;

private:
  /// Compositions can not be copied.
  Cog(const Cog&);
  void operator=(const Cog&);
};

String CogDisplayName(HandleParam object);

template<typename type>
inline type* Cog::Has()
{
  return (type*)QueryComponentType(ZilchTypeId(type));
}

template<typename type>
type* HasOrAdd(Cog* cog)
{
  type* component = cog->Has<type>();
  if (component == nullptr)
  {
    component = new type();
    SetUpObject(component);
    cog->AddComponent(component);
  }
  return component;
}

template <typename T>
void SetUpObject(T* object)
{
  BoundType* type = ZilchVirtualTypeId(object);
  CogComponentMeta* metaComponent = type->HasInherited<CogComponentMeta>();

  SetupMode::Enum constructionMode = metaComponent->mSetupMode;
  if (constructionMode == SetupMode::CallSetDefaults)
  {
    object->SetDefaults();
  }
  else if (constructionMode == SetupMode::DefaultSerialization)
  {
    DefaultSerializer defaultSerializer;
    object->Serialize(defaultSerializer);
  }
}

//------------------------------------------------------------------------------- Cog Handle Manager
class CogHandleData
{
public:
  void* mRawObject;
  CogId mCogId;
};

class CogHandleManager : public HandleManager
{
public:
  CogHandleManager(ExecutableState* state) : HandleManager(state) {}

  /// HandleManager interface.
  void Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags) override;
  void ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize) override;
  byte* HandleToObject(const Handle& handle) override;
  void Delete(const Handle& handle) override;
  bool CanDelete(const Handle& handle) override;
  size_t Hash(const Handle& handle) override;
};

}//namespace Zero
