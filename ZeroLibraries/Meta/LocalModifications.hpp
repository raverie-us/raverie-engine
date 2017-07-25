///////////////////////////////////////////////////////////////////////////////
///
/// \file LocalModifications.hpp
///
/// Authors: Joshua Claeys
/// Copyright 2015-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace ObjectContext
{
DeclareStringConstant(Instance);
}

// Forward declarations
class ObjectRestoreState;

//-------------------------------------------------------------------------- Contextual Object State
class ObjectState
{
public:
  //--------------------------------------------------------------------------------------- Child Id
  struct ChildId
  {
    explicit ChildId(StringParam typeName = "", Guid id = cInvalidUniqueId);
    ChildId(BoundType* type, Guid id = cInvalidUniqueId);
    ChildId(HandleParam object);
    ChildId(Object* object);

    size_t Hash() const;
    bool operator==(const ChildId& rhs) const;

    String mTypeName;
    Guid mId;
  };

  /// Typedefs.
  typedef HashSet<PropertyPath> ModifiedProperties;
  typedef HashSet<ChildId> ChildrenMap;
  typedef const ChildId& ChildIdParam;

  /// Constructor.
  ObjectState();

  ObjectState* Clone();
  void Combine(ObjectState* state);

  /// Checks both IsSelfModified and AreChildrenModified.
  bool IsModified();
  bool IsModified(HandleParam object, bool ignoreOverrideProperties);
  bool IsSelfModified();
  bool IsSelfModified(HandleParam object, bool ignoreOverrideProperties);
  bool AreChildrenModified();

  void ClearModifications();
  void ClearModifications(HandleParam object, bool retainOverrideProperties);
  /// The 'cachedData' is strictly for performance to save allocations when this function
  /// is called multiple times.
  void ClearModifications(bool retainOverrideProperties, HandleParam object,
                          ModifiedProperties& cachedMemory);

  /// Property modifications.
  bool IsPropertyModified(PropertyPathParam property);
  bool HasModifiedProperties();
  void SetPropertyModified(PropertyPathParam property, bool state);
  ModifiedProperties::range GetModifiedProperties();

  /// Child modifications.
  void ChildAdded(ChildIdParam childId);
  void ChildRemoved(ChildIdParam childId);
  bool IsChildLocallyAdded(ChildIdParam childId);
  bool IsChildLocallyRemoved(ChildIdParam childId);

  bool IsChildOrderModified();
  void SetChildOrderModified(bool state);

  ChildrenMap::range GetAddedChildren();
  ChildrenMap::range GetRemovedChildren();

  /// Properties
  ModifiedProperties mModifiedProperties;

  /// The engine uses these as both Hierarchy children and Components. They can be either
  /// type names or guids.
  ChildrenMap mAddedChildren;
  ChildrenMap mRemovedChildren;
  
  /// Whether or not the order of the children has changed.
  bool mChildOrderModified;
};

//----------------------------------------------------------------------- Local Object Modifications
class LocalModifications : public ExplicitSingleton<LocalModifications, Object>
{
public:
  ObjectState* GetObjectState(HandleParam object, bool createNew = false, bool validateStorage = true);

  /// Is the object modified in any way? Checks IsSelfModified and IsChildrenModified.
  bool IsModified(HandleParam object, bool checkHierarchy, bool ignoreOverrideProperties);

  /// Finds the closest parent (or itself) that inherits from other data. This will not recurse
  /// past parents that inherit from data. See the comment in the implementation of the
  /// 'MetaDataInheritance::ShouldStoreLocalModifications' function for a more detailed
  /// explanation.
  Handle GetClosestInheritedObject(HandleParam object, bool checkParents);

  /// Clear all modifications on the given object.
  void ClearModifications(HandleParam object, bool clearChildren, bool retainOverrideProperties);
  /// The 'cachedData' is strictly for performance to save allocations as it recurses.
  void ClearModifications(HandleParam object, bool clearChildren, bool retainOverrideProperties,
                          ObjectState::ModifiedProperties& cachedMemory);

  /// Property modifications.
  bool IsPropertyModified(HandleParam object, PropertyPathParam property);
  void SetPropertyModified(HandleParam object, PropertyPathParam property, bool state);

  /// Child modifications.
  void ChildAdded(HandleParam object, ObjectState::ChildIdParam childId);
  void ChildRemoved(HandleParam object, ObjectState::ChildIdParam childId);
  bool IsChildLocallyAdded(HandleParam object, ObjectState::ChildIdParam childId);
  bool IsObjectLocallyAdded(HandleParam object, bool recursivelyCheckParents);
  bool IsChildOrderModified(HandleParam object);
  void SetChildOrderModified(HandleParam object, bool state);

  /// Returns the object state stored for this object and removes it from the global map.
  ObjectState* TakeObjectState(HandleParam object);

  /// Updates the local modifications for the given object.
  void RestoreObjectState(HandleParam object, ObjectState* objectState);
  void CombineObjectState(HandleParam object, ObjectState* objectState);

private:
  bool IsModified(HandleParam object, HandleParam propertyPathParent,
                  bool checkHierarchy, bool ignoreOverrideProperties);
  HashMap<Handle, ObjectState*> mObjectStates;
};

//---------------------------------------------------------------------------- Meta Data Inheritance
DeclareEnum2(InheritIdContext,
  // This is used when saving a modified instance of an object (e.g. a Cog inside a level file).
  Instance,
  // This is used when saving out the definition of an object (e.g. uploading to Archetype). This
  // should only return the base inherit id (e.g. base archetype id).
  Definition);

// This is required IF you have the 'StoreLocalModifications' flag set on meta.
class MetaDataInheritance : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// A unique identifier for this object. This will be used 
  virtual Guid GetUniqueId(HandleParam object);

  /// This allows us to choose which objects we want to store local modifications.
  /// For example, we don't want to store changed for non-Archetyped Cogs or for
  /// Resources that don't inherit from other Resources.
  virtual bool ShouldStoreLocalModifications(HandleParam object);
  virtual void Revert(HandleParam object);
  virtual bool CanPropertyBeReverted(HandleParam object, PropertyPathParam propertyPath);
  virtual void RevertProperty(HandleParam object, PropertyPathParam propertyPath);
  virtual void SetPropertyModified(HandleParam object, PropertyPathParam propertyPath, bool state);

  /// When a modification of an object is being reverted, the object has to be rebuilt to
  /// properly get the old value. This function should basically re-create the object
  /// (i.e. Archetype being rebuilt).
  virtual void RebuildObject(HandleParam object) = 0;

  /// Needed??
  //virtual void SetChildOrderOverride(HandleParam object);
};

//---------------------------------------------------------------------------- Meta Data Inheritance
// Implement this if the object itself can inherit from other data (e.g. Cog, Material)
class MetaDataInheritanceRoot : public MetaDataInheritance
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// The id this object inherits from.
  virtual String GetInheritId(HandleParam object, InheritIdContext::Enum context) = 0;
  virtual void SetInheritId(HandleParam object, StringParam inheritId) = 0;
};

}//namespace Zero
