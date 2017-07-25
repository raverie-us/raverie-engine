////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys, Trevor Sundberg
/// Copyright 2016-2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------------------------------------------ Actions
DeclareEnum2(EnumerateAction,
  // All Components that could ever be dynamically added to the object type
  All,
  // All Components that can currently be added to the given object.
  // This does dependency checking.
  AllAddableToObject);

//----------------------------------------------------------------------------------- Meta Component
//SetupMode is used for dynamic addition setup
DeclareEnum4(SetupMode,
  FromDataOnly,         // Object can be setup from data only.
  DefaultConstructor,   // Default constructor will setup the object
  DefaultSerialization, // Default serialization will setup the object
  CallSetDefaults       // Calling SetDefaults will setup the object
);

// A meta component describes everything we need to know for a component.
// When accessing these on a component type, it should generally be with Has, not HasInherited.
class CogComponentMeta : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  CogComponentMeta(BoundType* owner);

  void AddInterface(BoundType* interfaceType);
  bool SupportsInterface(BoundType* interfaceMeta);

  /// Is 'component' dependent on 'dependent'.
  static bool IsDependentOn(BoundType* component, BoundType* dependent);

  SetupMode::Enum mSetupMode;
  HashSet<BoundType*> mDependencies;

  // Derived adds its base to its own Interfaces
  // (i.e. BoxCollider adds Collider to its own Interfaces)
  Array<BoundType*> mInterfaces;
  Array<BoundType*> mInterfaceDerivedTypes;
  BoundType* mOwner;
  HashSet<String> mTags;
};

//----------------------------------------------------------------------------------------- Add Info
/// Gives information about why a Component cannot be added.
struct AddInfo
{
  Handle BlockingComponent;
  BoundType* MissingDependency;
  String Reason;
};

//--------------------------------------------------------------------------------- Meta Composition
/// Meta Interface for polymorphic, dynamic containment of objects / components.
class MetaComposition : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  MetaComposition(BoundType* componentType);

  //---------------------------------------------------------------------------------------- Queries
  /// Returns how many Components the given instance has.
  virtual uint GetComponentCount(HandleParam owner) = 0;

  /// Returns whether or not the given object contains the given component type.
  virtual bool HasComponent(HandleParam owner, BoundType* componentType);

  /// Gets the component with the given type. Default behavior is to walk all components.
  /// This should be overridden to be more optimal on derived types.
  virtual Handle GetComponent(HandleParam owner, BoundType* componentType);

  /// Returns the component at the given index.
  virtual Handle GetComponentAt(HandleParam owner, uint index);

  /// Gets the component with the given unique id (child id). This allows multiple
  /// components of the same type to be referenced.
  virtual Handle GetComponentUniqueId(HandleParam owner, u64 uniqueId);

  /// Returns the index of the given Component type.
  virtual uint GetComponentIndex(HandleParam owner, BoundType* componentType);

  /// Returns the index of the given Component. By default, calls the pure virtual
  /// GetComponentIndex that takes the type. This could be overridden to be more optimal.
  virtual uint GetComponentIndex(HandleParam owner, HandleParam component);

  //----------------------------------------------------------------------------------- Modification
  /// Fills out the given array with all dynamically addable types. The owner is only required
  /// if querying for all addable to object.
  virtual void Enumerate(Array<BoundType*>& addTypes, EnumerateAction::Enum action,
                         HandleParam owner = nullptr);
  void Enumerate(Array<String>& addTypes, EnumerateAction::Enum action, HandleParam owner = nullptr);

  /// Checks dependencies to see if a component of the given type can be added.
  virtual bool CanAddComponent(HandleParam owner, BoundType* typeToAdd, AddInfo* info = nullptr);

  /// Creates an object of the given type.
  virtual Handle MakeObject(BoundType* typeToCreate);
  virtual BoundType* MakeProxy(StringParam typeName, ProxyReason::Enum reason);

  /// Allocates and adds the component to the object at the given index. Default index puts the
  /// component at the back of all components.
  virtual void AddComponent(HandleParam owner, HandleParam component, int index = -1,
                            bool ignoreDependencies = false);
  virtual void AddComponent(HandleParam owner, BoundType* typeToAdd, int index = -1,
                            bool ignoreDependencies = false); 

  /// Checks dependencies to see if the component can be removed. If it can't be removed, the reason
  /// will be filled out explaining why.
  virtual bool CanRemoveComponent(HandleParam owner, HandleParam component, String& reason);
  bool CanRemoveComponent(HandleParam owner, BoundType* componentType, String& reason);

  /// Removes the given component. Success or failure will be in the Status object. This function
  /// assumes that the operation has already been validated through 'CanRemoveComponent'.
  virtual void RemoveComponent(HandleParam owner, HandleParam component,
                               bool ignoreDependencies = false);

  /// Checks whether or not the given component can be moved to the given destination. If it cannot
  /// be moved because of a dependency, the infringing component will be returned.
  bool CanMoveComponent(HandleParam owner, HandleParam component, uint destination,
                        Handle& blockingComponent, String& reason);

  /// Attempts to move the given component to the given index. If it cannot be moved there due to
  /// moving before a dependency or after a dependent, it will return the blocking component.
  virtual void MoveComponent(HandleParam owner, HandleParam component, uint destination);

  //-------------------------------------------------------------------------------- Component Range
  struct ComponentRange
  {
    bool Empty();
    Handle Front();
    void PopFront();

    Handle mOwner;
    MetaComposition* mComposition;
    uint mIndex;
  };

  /// Range containing all children of the given object.
  ComponentRange AllComponents(HandleParam owner);

  /// Used in the property grid in the add component button
  virtual String GetAddName();

  /// The type of Component contained in the composition.
  BoundType* mComponentType;

  bool mSupportsComponentRemoval;
  bool mSupportsComponentReorder;
  
  HashMap<String, BoundType*> mComponentTypes;
};

//--------------------------------------------------------------------------------------- Meta Owner
typedef Handle(*GetOwnerFunction)(HandleParam object);

/// Used to generically get the owner of an object.
class MetaOwner : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  MetaOwner(GetOwnerFunction func) : mGetOwner(func) {}

  Handle GetOwner(HandleParam object);

  GetOwnerFunction mGetOwner;
};

}//namespace Zero
