////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// \file CogSerialization.hpp
/// 
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2010-2016, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
class Selection;
class Hierarchy;
class Space;
class MetaEventConnection;
class ObjectSaver;
class CachedModifications;

//-------------------------------------------------------------------------------- Cog Serialization
class CogSerialization
{
public:
  /// Checks all flags
  static bool ShouldSave(Cog& cog);
  static bool ShouldSave(Object* object);

  static void SaveSpaceToStream(Serializer& serializer, Space* space);
  static void SaveHierarchy(Serializer& serializer, Hierarchy* hierarchy);
  static void SaveSelection(Serializer& serializer, MetaSelection* selection);

  static void LoadHierarchy(Serializer& serializer, CogCreationContext* context, Hierarchy* hierarchy);

  static String SaveToStringForCopy(Cog* cog);

  /// Copy + pasting an object is done by saving out the object to text, then loading from that text.
  /// If we're copy + pasting an Archetype that's a child of another Archetype, there are some
  /// extra steps we have to do to guarantee the object that is pasted has the same exact properties
  /// as the one copied. Call these two functions (one before, one after) saving the object to
  /// text.
  /// PreProcess returns whether or not a post process is required after the object is saved.
  static bool PreProcessForCopy(Cog* cog, CachedModifications& restoreState);
  static void PostProcessAfterCopy(Cog* cog, CachedModifications& restoreState);
};

//------------------------------------------------------------------------------------------- LinkId
///Component used to resolve links between saved objects in data files.
///component only on compositions serialized to data file. At runtime this info
///is stored on the compositions CogId member.
class LinkId : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;

  uint Id;
};

//-------------------------------------------------------------------------------------------- Named
///Component used to store an game object's name.
///Component only on compositions serialized to data file. At runtime this info
///is stored on the compositions Name member.
class Named : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;

  String Name;
};

//--------------------------------------------------------------------------------------- Archetyped
class Archetyped : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;

  String Name;
};

//------------------------------------------------------------------------------------- Editor Flags
class EditorFlags : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  EditorFlags() {}
  ~EditorFlags() {}

  //Component Interface
  void Serialize(Serializer& stream) override;

  bool mLocked;
  bool mHidden;
};

//------------------------------------------------------------------------------------ Space Objects
class SpaceObjects : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface
  void Serialize(Serializer& stream) override;

  Space* mSpace;
};

//------------------------------------------------------------------------------- Archetype Instance
/// Used as an alternate to data tree patching. This is strictly an optimization.
class ArchetypeInstance
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  String Name;
  uint Version;
};

// A function pointer that we use for checking a component should be serialized
DeclareEnum2(SerializeCheck, Serialized, NotSerialized);
typedef SerializeCheck::Enum(*ComponentCheckFn)(Cog* composition, Component* component);

DeclareEnum2(ContextMode, Creating, Saving);

//------------------------------------------------------------------------ Cog Serialization Context
class CogSerializationContext
{
public:
  //Shared Flags
  uint Flags;

  //Context Mode
  ContextMode::Enum CurrentContextMode;
};

const u32 cContextIdMask = 0xFFFF0000;

//------------------------------------------------------------------------------- Cog Saving Context
class CogSavingContext : public CogSerializationContext
{
public:
  CogSavingContext();
  uint ToContextId(uint objectId);

  // Counter used to generate ids.
  uint CurrentId;

  // Map of scene ids to  ContextIds.
  HashMap<uint, uint> ContextIdMap;

  // Doesn't need to be a handle because Resources will never be deleted while saving Cogs.
  Archetype* SavingArchetype;

  // A callback that can be provided to ignore certain components
  // (if NULL, the callback is ignored)
  ComponentCheckFn ShouldSerializeComponentCallback;
};

//----------------------------------------------------------------------------- Cog Creation Context
class CogCreationContext : public CogSerializationContext
{
public:
  typedef Array<MetaEventConnection*> EventConnections;

  CogCreationContext();
  CogCreationContext(Space* space, StringParam source);

  // Object created in this context
  struct CreationEntry
  {
    CreationEntry() : NewId(0), Object(nullptr) {};
    CreationEntry(uint newId, Cog* obj)
      : NewId(newId), Object(obj) {}
    uint NewId;
    Cog* Object;
  };

  /// Enters a sub context. Should be called before loading an Archetype.
  /// Returns mCurrentSubContextId before changing it. This value should be
  /// passed into the LeaveSubContext function.
  uint EnterSubContext();

  /// Restores to the given sub context id. Should be called after loading
  /// an Archetype.
  void LeaveSubContext(uint previousSubContextId);

  /// Adds the cog to the context map and assigns a context id.
  void RegisterCog(Cog* cog, uint localContextId);

  /// Assigns the current sub context id to the given Cog. This is used
  /// to later resolve CogPath properties on the Cog.
  void AssignSubContextId(Cog* cog);

  /// In a single Archetype, Cogs are assigned incremental local context
  /// id's (starting at 1). When we load multiple Archetypes at the same
  /// time (same context), their id's will overlap.
  ///
  /// Every time we start loading an Archetype, we need to assign unique id's
  /// to the Cogs in the Archetype. To do this, we store an id in the upper
  /// 16 bits of mCurrentSubContextId that will be added to the local context
  /// id on each object.
  uint mCurrentSubContextId;

  /// Used to generate unique values for mCurrentSubContextId. This value
  /// increments every time we start loading an Archetype.
  /// 
  /// Example when going into a new Archetype:
  /// ++mSubIdCounter;
  /// mCurrentSubContextId = (mSubIdCounter << 16);
  uint mSubIdCounter;

  // Map of serialized ContextId to CreationEntry
  typedef HashMap<uint, CreationEntry> IdMapType;
  IdMapType mContextIdMap;

  EventConnections EventToMaps;

  //Where is this data being loaded from.
  String Source;

  //Space the objects are being created in.
  Space* mSpace;

  //GameSession the objects/spaces are being created in.
  GameSession* mGameSession;
};

}//namespace Zero
