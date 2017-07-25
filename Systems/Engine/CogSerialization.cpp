////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// \file CogSerialization.cpp
/// 
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2010-2016, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------------------- Cog Serialization
//**************************************************************************************************
bool CogSerialization::ShouldSave(Cog& cog)
{
  uint flagsStopSaving = CogFlags::Transient | CogFlags::Destroyed;

  if((cog.mFlags.U32Field & flagsStopSaving) != 0)
    return false;

  if(cog.GetId() == cInvalidCogId)
    return false;

  return true;
}

//**************************************************************************************************
bool CogSerialization::ShouldSave(Object* object)
{
  Cog* cog = (Cog*)object;
  return ShouldSave(*cog);
}

//**************************************************************************************************
Cog* ResolveCog(Cog& r) { return &r; }
Cog* ResolveCog(Cog* r) { return r; }

template<typename rangeType>
void SaveRangeOfObjects(Serializer& saver, uint objectCount, rangeType range)
{
  // Should fix this
  ObjectSaver* objectSaver = (ObjectSaver*)&saver;

  rangeType final = range;

  uint shouldSaveCount = 0;
  while(!range.Empty())
  {
    Cog* cog = ResolveCog(range.Front());
    if(ShouldSave(*cog))
      ++shouldSaveCount;
    range.PopFront();
  }

  objectSaver->ArraySize(shouldSaveCount);

  range = final;
  while(!range.Empty())
  {
    Cog* cog = ResolveCog(range.Front());
    objectSaver->SaveInstance(cog);
    range.PopFront();
  }
}

//**************************************************************************************************
void CogSerialization::SaveSpaceToStream(Serializer& saver, Space* space)
{
  SaveRangeOfObjects(saver, space->mRootCount, space->mRoots.All());
}

//**************************************************************************************************
void CogSerialization::SaveHierarchy(Serializer& serializer, Hierarchy* hierarchy)
{
  uint count = 0;
  for(HierarchyList::range r = hierarchy->Children.All(); !r.Empty(); r.PopFront())
    ++count;

  SaveRangeOfObjects(serializer, count, hierarchy->Children.All());
}

//**************************************************************************************************
void CogSerialization::SaveSelection(Serializer& saver, MetaSelection* selection)
{
  Array<Cog*> cogs;
  FilterChildrenAndProtected(cogs, selection);

  size_t cogCount = cogs.Size();

  // If a cog is an Archetype, and is a child of another Archetype, we need to save out all of
  // our modifications in our parent's Archetype
  // Do this for each object that we're copying
  typedef Pair<bool, CachedModifications> CacheEntry;
  Array<CacheEntry> cachedModifications;
  cachedModifications.Resize(cogCount);

  for(uint i = 0; i < cogCount; ++i)
  {
    Cog* cog = cogs[i];
    CacheEntry& entry = cachedModifications[i];
    entry.first = PreProcessForCopy(cog, entry.second);
  }

  saver.StartPolymorphic("Selection");
  SaveRangeOfObjects(saver, cogs.Size(), cogs.All());
  saver.EndPolymorphic();

  for (uint i = 0; i < cogCount; ++i)
  {
    Cog* cog = cogs[i];
    CacheEntry& entry = cachedModifications[i];
    if (entry.first)
      PostProcessAfterCopy(cog, entry.second);
  }
}

//**************************************************************************************************
void CogSerialization::LoadHierarchy(Serializer& serializer, CogCreationContext* context, Hierarchy* hierarchy)
{
  bool hadCogs = serializer.Start("Cogs", "cogs", StructureType::Object);
  Cog* parent = hierarchy->GetOwner();

  uint numberOfObjects = 0;
  serializer.ArraySize(numberOfObjects);
  for(uint i = 0; i < numberOfObjects; ++i)
  {
    Cog* cog = Z::gFactory->BuildFromStream(context, serializer);
    if(cog == nullptr)
      continue;

    // Prevent composition from adding to parent twice and work around
    // for separating serialization and initialization.
    cog->mHierarchyParent = parent;
    hierarchy->Children.PushBack(cog);

  }

  if(hadCogs)
    serializer.End("Cogs", StructureType::Object);
}

//**************************************************************************************************
String CogSerialization::SaveToStringForCopy(Cog* cog)
{
  CogSavingContext context;

  // If this cog is an Archetype, and is a child of another Archetype, we need to save out all of
  // our modifications in our parent's Archetype
  CachedModifications restoreState;
  bool modificationsApplied = PreProcessForCopy(cog, restoreState);

  // Save to a string
  ObjectSaver saver;
  saver.OpenBuffer();
  saver.SetSerializationContext(&context);
  saver.SaveInstance(cog);

  // Restore our old modifications
  if (modificationsApplied)
    PostProcessAfterCopy(cog, restoreState);

  // Extract the data
  return saver.GetString();
}

//**************************************************************************************************
bool CogSerialization::PreProcessForCopy(Cog* cog, CachedModifications& restoreState)
{
  // If we're an Archetype, and a child of another Archetype, there's one extra step we need to take
  // before saving the object
  Cog* archetypeContextCog = cog->FindNearestArchetypeContext();
  if (archetypeContextCog && archetypeContextCog != cog)
  {
    // We're going to make modifications to our current state so that we can save out properly.
    // Store out our state so we can recover it after saving
    restoreState.Cache(cog);

    // Apply all modifications to us in our parents Archetype
    Archetype* archetypeContext = archetypeContextCog->GetArchetype();
    CachedModifications& cachedModifications = archetypeContext->GetAllCachedModifications();

    // We want to retain our local modifications
    bool combine = true;
    cachedModifications.ApplyModificationsToChildObject(archetypeContextCog, cog, combine);

    return true;
  }

  return false;
}

//**************************************************************************************************
void CogSerialization::PostProcessAfterCopy(Cog* cog, CachedModifications& restoreState)
{
  // Clear before restoring
  LocalModifications::GetInstance()->ClearModifications(cog, true, false);
  restoreState.ApplyModificationsToObject(cog);
}

//---------------------------------------------------------------------- Link Id
//******************************************************************************
ZilchDefineType(LinkId, builder, type)
{
}

//******************************************************************************
void LinkId::Serialize(Serializer& stream)
{
  SerializeName(Id);
}

//------------------------------------------------------------------------ Named
//******************************************************************************
ZilchDefineType(Named, builder, type)
{
}

//******************************************************************************
void Named::Serialize(Serializer& stream)
{
  SerializeName(Name);
}

//------------------------------------------------------------------- Archetyped
//******************************************************************************
ZilchDefineType(Archetyped, builder, type)
{
}

//******************************************************************************
void Archetyped::Serialize(Serializer& stream)
{
  SerializeName(Name);
}

//----------------------------------------------------------------- Editor Flags
//******************************************************************************
ZilchDefineType(EditorFlags, builder, type)
{
}

//******************************************************************************
void EditorFlags::Serialize(Serializer& stream)
{
  SerializeNameDefault(mLocked, false);
  SerializeNameDefault(mHidden, false);
}

ZilchDefineType(ArchetypeInstance, builder, type)
{
}

Cog* resolve(Cog& r) { return &r; }
Cog* resolve(Cog* r) { return r; }

//******************************************************************************
bool ShouldSave(Cog& cog)
{
  uint flagsStopSaving = CogFlags::Transient | CogFlags::Destroyed;

  if((cog.mFlags.U32Field & flagsStopSaving) != 0)
    return false;

  if(cog.GetId() == cInvalidCogId)
    return false;

  return true;
}

ZilchDefineType(SpaceObjects, builder, type)
{
}

//******************************************************************************
void SpaceObjects::Serialize(Serializer& stream)
{
  CogSerialization::SaveSpaceToStream(stream, mSpace);
}

CogCreationContext::CogCreationContext()
{
  mCurrentSubContextId = 0;
  mSubIdCounter = 0;
  Flags = 0;
  mSpace = nullptr;
  mGameSession = nullptr;
  CurrentContextMode = ContextMode::Creating;
}

CogCreationContext::CogCreationContext(Zero::Space* space, StringParam source)
{
  mCurrentSubContextId = 0;
  mSubIdCounter = 0;
  Flags = 0;
  mSpace = space;
  mGameSession = space->GetGameSession();
  Source = source;
  CurrentContextMode = ContextMode::Creating;
}

//******************************************************************************
uint CogCreationContext::EnterSubContext()
{
  // Store the current id before modification to return it
  uint previousSubContextId = mCurrentSubContextId;

  // Move to the next context id
  ++mSubIdCounter;

  // The id should be stored in the high bits
  mCurrentSubContextId = (mSubIdCounter << 16);

  return previousSubContextId;
}

//******************************************************************************
void CogCreationContext::LeaveSubContext(uint previousSubContextId)
{
  mCurrentSubContextId = previousSubContextId;
}

//******************************************************************************
void CogCreationContext::RegisterCog(Cog* cog, uint localContextId)
{
  CogId id = cog->GetId();
  CreationEntry entry(id.Id, cog);

  // Apply the sub context id before inserting
  uint contextId = localContextId + mCurrentSubContextId;
  mContextIdMap.Insert(contextId, entry);

  if(false)
  {
    u64 id64 = contextId;
    String hexId = ToString(id64, false).SubStringFromByteIndices(8, 16);
    DebugPrint("Storing ContextId: %s (%u) on \"%s\"\n", hexId.c_str(), contextId, cog->mName.c_str());
  }

  // The sub
  AssignSubContextId(cog);
}

//******************************************************************************
void CogCreationContext::AssignSubContextId(Cog* cog)
{
  // Archetypes will be registered twice, once with each the inner and outer
  // context. The SubContext Id should only be set once
  if(cog->mSubContextId == 0)
  {
    cog->mSubContextId = mCurrentSubContextId;

    if(false)
    {
      u64 id64 = mCurrentSubContextId;
      String hexId = ToString(id64, false).SubStringFromByteIndices(8, 16);
      DebugPrint("Setting Cog.SubContextId: %s (%u) on \"%s\"\n", hexId.c_str(), cog->mSubContextId, cog->mName.c_str());
    }
  }
}

CogSavingContext::CogSavingContext()
{
  Flags = 0;
  CurrentId = 0;
  CurrentContextMode = ContextMode::Saving;
  ShouldSerializeComponentCallback = nullptr;
  SavingArchetype = nullptr;
}

uint CogSavingContext::ToContextId(uint objectId)
{
  //See if the id has already been saved
  uint contextId = ContextIdMap.FindValue(objectId, 0);
  if(contextId != 0)
    return contextId;
  else
  {
    // If the id has not been saved
    // generate a new id and Assign it.
    ++CurrentId;
    ContextIdMap.Insert(objectId, CurrentId);
    return CurrentId;
  }
}

//template<typename rangeType>
//void SerializeRangeOfObjects(Serializer& saver, uint objectCount, rangeType range)
//{
//  SaveRangeOfObjects(saver, objectCount, range, CogSerialization::SerializeObject);
//}
//
//template<typename rangeType>
//void SerializeRangeOfStartObjects(Serializer& saver, uint objectCount, rangeType range)
//{
//  SaveRangeOfObjects(saver, objectCount, range, CogSerialization::StartSerializeObject);
//}

}//namespace Zero
