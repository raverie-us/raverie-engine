///////////////////////////////////////////////////////////////////////////////
///
/// \file Space.hpp
/// Declaration the Space component class.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
typedef InList<Cog, &Cog::SpaceLink> SpaceCogList;
typedef InList<Cog, &Cog::NameLink> NameCogList;

namespace Events
{
  DeclareEvent(SpaceLevelLoaded);
  DeclareEvent(SpaceModified);
  DeclareEvent(SpaceObjectsChanged);
  DeclareEvent(SpaceDestroyed);
}//namespace Events

// Type define for a range
typedef NameCogList::range CogNameRange;

DeclareBitField4(CreationFlags,
  Editing,
  DynamicallyAdded,
  Preview,
  /// Are proxy components expected when creating this object? Doesn't warn on proxies being created.
  ProxyComponentsExpected);

namespace CreationFlags
{
  const CreationFlags::Enum Default = (CreationFlags::Enum)0;
}//namespace CreationFlags

DeclareBitField1(DestroyFlags, DynamicallyDestroyed);

//------------------------------------------------------------------------ Space

/// A space is a near boundless, three-dimensional extent in which objects 
/// and events occur and have relative position, direction, and time.
/// Essentially a world of objects that exist together.
/// Used to divide objects between UI, World, Editor, and others. The two most
/// Common spaces are the 'World' for the game world and the 'Ui'
/// for the HUD and menus.
class Space : public Cog
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  IntrusiveLink(Space, link);
  typedef SpaceCogList::range range;

  Space();
  ~Space();

  // Cog Interface
  void Initialize(CogInitializer& initializer) override;
  void SetName(StringParam newName) override;
  bool IsEditorMode() override;
  bool IsPreviewMode() override;
  bool IsEditorOrPreviewMode() override;
  Cog* Clone() override;

  // Get Space on Space returns itself
  Space* GetSpace() override;
  GameSession* GetGameSession() override;

  //----------------------------------------------------------------- Creation

  /// Create an object in the space
  Cog* Create(Archetype* archetype);

  /// Create a object at a position in the space
  Cog* CreateAtPosition(Archetype* archetype, Vec3Param position);

  /// Create an object from an archetype.
  Cog* CreateNamed(StringParam archetypeName, StringParam name = String());

  Cog* CreateAt(StringParam source, Transform* transform);
  Cog* CreateAt(StringParam source, Vec3Param position);
  Cog* CreateAt(StringParam source, Vec3Param position, QuatParam rotation);
  Cog* CreateAt(StringParam source, Vec3Param position, Vec3Param scale);
  Cog* CreateAt(StringParam source, Vec3Param position, QuatParam rotation, 
                Vec3Param scale);

  //Create an object link between two objects
  Cog* CreateLink(Archetype* archetype, Cog* objectA, Cog* objectB);
  Cog* CreateNamedLink(StringParam archetypeName, Cog* objectA, Cog* objectB);
  
  //------------------------------------------------------------------ Loading
  /// Load new level replace the current level.
  void LoadLevel(Level* level);

  /// Reload the current level.
  void ReloadLevel();

  /// Save a level file.
  void SaveLevelFile(StringParam levelName);

  /// Last level loaded.
  Level* GetCurrentLevel();

  /// Load the pending level. Called before update.
  void LoadPendingLevel();

  /// Load the level file
  void LoadLevelFile(StringParam filePath);

  //----------------------------------------------------------- Adding Objects

  /// Do no destroy current objects, add objects from level and change loaded level.
  void LoadLevelAdditive(Level* levelName);

  /// Add all objects from a level.
  Level* AddObjectsFromLevel(Level* levelName);

  /// Add objects from serializer stream.
  range AddObjectsFromStream(StringParam source, Serializer& stream);

  //------------------------------------------------------- Destroying Objects
  /// Destroy all objects in space.
  void DestroyAll();

  /// Destroy all objects created from level.
  void DestroyAllFromLevel();

  /// Destroy the space and all objects inside it.
  void Destroy() override;

  /// Internal Destroy
  void ForceDestroy() override;

  //---------------------------------------------------------------- Objects

  /// Find an object in the space with a given name.
  CogNameRange FindAllObjectsByName(StringParam name);

  /// Find an object in the space with a given name.
  Cog* FindObjectByName(StringParam name);
  Cog* FindFirstObjectByName(StringParam name);
  Cog* FindLastObjectByName(StringParam name);
  Cog* FindFirstRootObjectByName(StringParam name);
  Cog* FindLastRootObjectByName(StringParam name);

  /// All objects in the space.
  range AllObjects() { return mCogList.All(); }

  /// Number of objects in the space.
  uint GetObjectCount() { return mCogsInSpace; }

  //------------------------------------------------------------ Modification

  //Any change that needs to be saved marks the space as modified.
  void MarkModified();
  bool GetModified();
  void MarkNotModified();

  /// Any change to the count / structure of the objects.
  void CheckForChangedObjects();
  /// This should be called whenever we want the object view to be refreshed / updated.
  void ChangedObjects();

  //-------------------------------------------------------------------- Flags
  //Get create flags used for create new objects in this space.
  //Used by Factory.
  uint GetCreationFlags();

  HierarchyList::range AllRootObjects(){return mRoots.All();}

//Internals
  void AddObject(Cog* cog);
  void RemoveObject(Cog* cog);
  typedef HashMap<String, NameCogList*> CogNameMap;
  CogNameMap mNameMap;

  static Memory::Pool* sCogLists;

  void AddToNameMap(Cog* cog, StringParam name);
  void RemoveFromNameMap(Cog* cog, StringParam name);

  // These two variables are used to guard against floating point exceptions.
  // If an object's position is larger than the max position then the value
  // is clamped and InvalidObjectPosition is set to true. This lets the engine
  // know to only display 1 error message per space for this.
  float mMaxObjectPosition;
  bool mInvalidObjectPositionOccurred;

  // Last level loaded into the space
  HandleOf<Level> mLevelLoaded;

  // The game session that created us (can be null)
  GameSession* mGameSession;

  // Internal
  // Objects
  SpaceCogList mCogList;
  uint mCogsInSpace;

  // Hierarchy
  HierarchyList mRoots;
  uint mRootCount;
  
  // If valid a load is pending for next update
  HandleOf<Level> mPendingLevel;
  // Allows CameraViewports to attach viewport to a space specific GameWidget
  HandleOf<GameWidget> mGameWidgetOverride;

  // When editing and state change will mark the level as modified.
  bool mModified;
  // Have the objects contained in the level changed. Use for editor ui.
  bool mObjectsChanged;
  // Loaded for editing use proxies
  BitField<CreationFlags::Enum> mCreationFlags;
  bool mSubSpace;
  // Is the space currently in the process of loading a level right now.
  bool mIsLoadingLevel;

  void SerializeObjectsToSpace(CogInitializer& initializer, 
                               CogCreationContext& context, Serializer& loader);

  friend class Cog;
  friend class CogInitializer;
  friend class SubSpaceMount;
  friend class Level;
  friend class SpaceObjectSource;
  friend class ArchetypeRebuilder;
};

}//namespace Zero
