///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum2(RegisteredGroupInstanceAccessMode, UseDefault, ReturnNull);

//-------------------------------------------------------------------CollisionTable
/// Defines filter pairs between CollisionGroups. These filters are used to control
/// if collision detection and resolution happens between Colliders. Additionally,
/// CollisionFilterBlocks can be defined on filters to send out extra events.
class CollisionTable : public DataResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  typedef CollisionFilter value_type;

  CollisionTable();
  ~CollisionTable();

  // DataResource Interface
  void Serialize(Serializer& stream) override;
  void Save(StringParam filename) override;
  void Unload() override;

  // Runtime interface
  /// Creates a CollisionTable for run-time modifications.
  static HandleOf<CollisionTable> CreateRuntime();
  HandleOf<Resource> Clone() override;
  /// Creates a clone of this CollisionTable for run-time modifications.
  HandleOf<CollisionTable> RuntimeClone();
  void CopyTo(CollisionTable* destination);

  void Initialize();
  void SetDefaults();
  /// Registers all groups in existence. Used to auto-populate newly created filters.
  void LoadExistingGroups();

  void ValidateFilters();

  // Clear all groups and filters from this table
  void Clear();
  /// Registers the group as usable for this space (max 32).
  void RegisterGroup(CollisionGroup* group);
  /// Removes the group from the usable type list so more can be added.
  /// Also removes all instances of the given type.
  void UnRegisterGroup(CollisionGroup* group);
  /// Finds the instance of a collision group. The user can specify what they
  /// want to have happen when the instance is not found via the access mode.
  CollisionGroupInstance* GetGroupInstance(ResourceId groupId,
    RegisteredGroupInstanceAccessMode::Enum accessMode = RegisteredGroupInstanceAccessMode::UseDefault);
  /// Finds the filter between the pair of collision groups.
  CollisionFilter* FindFilter(CollisionFilter& pair);
  /// Finds the filter between the pair of collision groups.
  CollisionFilter* FindFilter(CollisionGroup* groupA, CollisionGroup* groupB);

  /// Helper function to remove all instances in the
  /// filters of the given collision group type.
  void RemoveGroupInstancesAndFilters(ResourceId groupId);

  void ReconfigureGroups();
  void FixSpaces(CollisionTable* newTable);

  CollisionGroup* GetDefaultGroup();

  typedef HashMap<ResourceId, CollisionGroupInstance*> RegisteredGroups;
  RegisteredGroups mRegisteredGroups;

  typedef HashSet<CollisionFilter*, CollisionFilterHashPolicy> HashedFilters;
  typedef Array<CollisionFilter*> CollisionFilters;

  /// The filters between the registered groups in a linear format for serialization.
  CollisionFilters mCollisionFilters;
  /// Quick lookup of filters by group pairs (the filter itself)
  HashedFilters mHashedFilters;
  bool mAutoRegister;
};

//-------------------------------------------------------------------CollisionTableManager
class CollisionTableManager : public ResourceManager
{
public:
  DeclareResourceManager(CollisionTableManager, CollisionTable);

  CollisionTableManager(BoundType* resourceType);
  CollisionTable* CreateNewResourceInternal(StringParam name) override;

  /// Manage a new CollisionGroup being created (to add to all existing tables)
  void OnCollisionGroupAdded(ResourceEvent* event);
  /// Manage a new CollisionGroup being removed (to remove from all existing tables)
  void OnCollisionGroupRemoved(ResourceEvent* event);
};

}//namespace Zero
