///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorOperations.cpp
/// Implementation of the PropertyOperation class.
/// 
/// Authors: Joshua Claeys, Ryan Edgemon
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"


namespace Zero
{

//******************************************************************************
template<typename type>
u64 GetId(type* change)
{
  MetaType* meta = change->GetMeta();
  MetaHandleData handle;
  meta->ObjectToHandle(meta, &handle, change);
  return handle.HandleId;
}

//******************************************************************************
void AttachObject(OperationQueue* queue, Cog* object, Cog* parent, bool relative)
{
  //Do not queue protected
  if(object->mFlags.IsSet(CogFlags::Protected))
    return;

  // When re-attaching to ourself, just move it to the end
  if(object->GetParent() == parent)
  {
    uint newIndex = parent->GetChildCount() - 1;
    MoveObjectIndex(queue, object, newIndex);
    return;
  }

  ErrorIf(object == nullptr || parent == nullptr, "Invalid objects given to AttachObject");

  queue->BeginBatch();
  queue->SetActiveBatchName(BuildString("'", CogDisplayName(object),
    "' attached to '", CogDisplayName(parent), "'"));

  // If it already has a parent, detach it first
  if(object->GetParent())
    DetachObject(queue, object);

  AttachOperation* op = new AttachOperation(object, parent, relative);
  op->Redo();
  queue->Queue(op);
  queue->EndBatch();
}

//******************************************************************************
void DetachObject(OperationQueue* queue, Cog* object, bool relative)
{
  //Do not queue protected
  if(object->mFlags.IsSet(CogFlags::Protected) || object == NULL)
    return;

  Cog* parent = object->GetParent();
  if(parent == NULL)
    return;

  DetachOperation* op = new DetachOperation(object, relative);
  op->Redo();
  queue->Queue(op);
}

//******************************************************************************
void MoveObjectIndex(OperationQueue* queue, Cog* objectToMove, uint index)
{
  // Queue the reorder operation
  ReorderOperation* op = new ReorderOperation(objectToMove, index);
  op->Redo();
  queue->Queue(op);
}

//******************************************************************************
void MoveObject(OperationQueue* queue, Cog* objectToMove, Cog* newParent,
                uint indexInNewParent, bool relativeAttach)
{
  ErrorIf(objectToMove == nullptr, "Invalid object given to MoveObject");

  queue->BeginBatch( );

  String objectName = CogDisplayName(objectToMove);
  // Get the name of the parent cog (could be null).
  String parentName = (newParent == nullptr) ? "Root" : CogDisplayName(newParent);

  queue->SetActiveBatchName(BuildString("'", objectName, "' moved to '", parentName, "'"));

  // If the object isn't already a child of the given parent, we have to attach it
  // If the new parent is NULL, we need to detach it
  if(objectToMove->GetParent() != newParent)
  {
    if(newParent)
      AttachObject(queue, objectToMove, newParent, relativeAttach);
    else
      DetachObject(queue, objectToMove);
  }

  // Move the object
  MoveObjectIndex(queue, objectToMove, indexInNewParent);

  queue->EndBatch();
}

//******************************************************************************
void DestroyObject(OperationQueue* queue, Cog* object)
{
  //Do not queue protected
  if(object->mFlags.IsSet(CogFlags::Protected))
    return;

  CreateDestroyOperation* op = new CreateDestroyOperation(object, ObjectOperationMode::Destroy);
  op->Redo();
  queue->Queue(op);
}

//******************************************************************************
void ObjectCreated(OperationQueue* queue, Cog* object)
{
  //Do not queue protected
  if(object->mFlags.IsSet(CogFlags::Protected))
    return;

  CreateDestroyOperation* op = new CreateDestroyOperation(object, ObjectOperationMode::Create);

  // Mark the space as modified
  if (Space* space = object->GetSpace())
    space->MarkModified();

  if(Cog* parent = object->GetParent())
  {
    LocalModifications* modifications = LocalModifications::GetInstance();
    modifications->ChildAdded(parent->has(Hierarchy), object);
  }
  queue->Queue(op);
}

//******************************************************************************
Cog* CreateFromArchetype(OperationQueue* queue, Space* space,
                         Archetype* archetype, Vec3Param translation,
                         QuatParam rotation, Vec3Param scale)
{
  // Only cog archetypes work with undo
  if (archetype->mStoredType != ZilchTypeId(Cog))
  {
    DoNotifyError("Invalid archetype", "Can not create this type of archetype in a space");
    return NULL;
  }

  CreateFromArchetypeOperation* create = new CreateFromArchetypeOperation(space,
    archetype, translation, rotation, scale);

  Cog* newObject = create->DoCreation();

  queue->Queue(create);

  return newObject;
}

//******************************************************************************
Cog* CreateFromArchetype(OperationQueue* queue, Space* space,
                         StringParam source, Vec3Param translation,
                         QuatParam rotation, Vec3Param scale)
{
  Archetype* archetype = ArchetypeManager::FindOrNull(source);
  if(archetype == NULL)
    return NULL;
  return CreateFromArchetype(queue, space, archetype,
                             translation, rotation, scale);
}

//******************************************************************************
void UploadToArchetype(OperationQueue* queue, Cog* cog)
{
  UploadToArchetypeOperation* op = new UploadToArchetypeOperation(cog);
  op->CacheArchetype(cog->GetArchetype());
  op->Redo();
  queue->Queue(op);
}

//******************************************************************************
Archetype* UploadToArchetype(OperationQueue* queue, Cog* cog, StringParam archetypeName,
                             Archetype* baseArchetype)
{
  // Create the operation to cache the current cog before making any changes
  UploadToNewArchetypeOperation* op = new UploadToNewArchetypeOperation(cog);
  
  ArchetypeManager* manager = ArchetypeManager::GetInstance();
  Archetype* newArchetype = manager->FindOrNull(archetypeName);
  if(newArchetype == nullptr)
  {
    Archetype* oldArchetype = cog->GetArchetype();
  
    // Create a new Archetype if it didn't exist
    newArchetype = manager->MakeNewArchetypeWith(cog, archetypeName, 0, baseArchetype);
    op->mUploadedToNewArchetype = true;
    op->mNewArchetype = newArchetype;

    op->Redo();
  
    // Copy over tags to the new Archetype
    if(oldArchetype)
    {
      TagList oldTags;
      oldArchetype->GetTags(oldTags);
      newArchetype->mContentItem->SetTags(oldTags);
    }
  }
  else
  {
    op->mNewArchetype = newArchetype;

    // Cache the Archetype we're changing to before uploading it
    op->CacheArchetype(newArchetype);
    op->Redo();
  }
  
  queue->Queue(op);
  
  return newArchetype;
}

//******************************************************************************
void RevertToArchetype(OperationQueue* queue, Cog* cog)
{
  Operation* op = new RevertToArchetypeOperation(cog);
  op->Redo();
  queue->Queue(op);
}

//******************************************************************************
void ClearArchetype(OperationQueue* queue, Cog* cog)
{
  Operation* op = new ClearArchetypeOperation(cog);
  op->Redo();
  queue->Queue(op);
}

//---------------------------------------------- Create From Archetype Operation
//******************************************************************************
CreateFromArchetypeOperation::CreateFromArchetypeOperation(Space* space,
                              Archetype* archetype, Vec3Param location,
                              QuatParam rotation, Vec3Param scale)
{
  mSpace = space;
  mSpaceWasModified = space->GetModified();
  mArchetype = archetype;
  mLocation = location;
  mRotation = rotation;
  mScale = scale;

  mName = BuildString("Create from \"", archetype->Name, "\" Archetype");
}

//******************************************************************************
void CreateFromArchetypeOperation::Undo()
{
  Space* space = mSpace;
  if(space == NULL)
    return;

  if(Cog* object = mUndoHandle)
    object->Destroy();

  space->ChangedObjects();
  if(mSpaceWasModified)
    space->MarkModified();
  else
    space->MarkNotModified();
}

//******************************************************************************
void CreateFromArchetypeOperation::Redo()
{
  DoCreation();
}

//******************************************************************************
Cog* CreateFromArchetypeOperation::DoCreation()
{
  Archetype* archetype = mArchetype;
  Space* space = mSpace;
  if(archetype == NULL || space == NULL)
    return NULL;

  Cog* object = NULL;
  if(mScale.LengthSq() == 0.0f)
    object = space->CreateAt(archetype->ResourceIdName, mLocation, mRotation);
  else
    object = space->CreateAt(archetype->ResourceIdName, mLocation, mRotation, mScale);

  if(object == NULL)
  {
    DoNotifyError("Archetype Error", "Failed to create object from archetype");
    return NULL;
  }

  object->MarkTransformModified();
  space->ChangedObjects();
  space->MarkModified();

  // Update the cog in the undo map
  mUndoHandle.UpdateObject(object);

  // Update all components in the undo map
  uint componentCount = object->GetComponentCount();
  mComponentHandles.Resize(componentCount, cInvalidUndoObjectId);

  for(uint i = 0; i < componentCount; ++i)
  {
    Component* component = object->GetComponentByIndex(i);
    mComponentHandles[i] = Z::gUndoMap->UpdateUndoId(mComponentHandles[i], component);
  }

  return object;
}

//--------------------------------------------------------------------------------- Attach Operation
//**************************************************************************************************
AttachOperation::AttachOperation(Cog* object, Cog* newParent, bool relativeAttach) :
  mNewChildId(PolymorphicNode::cInvalidUniqueNodeId),
  mRelativeAttach(relativeAttach),
  mObject(object),
  mParent(newParent)
{
  ErrorIf(object == nullptr || newParent == nullptr, "Invalid objects given to AttachOperation");

  mName = BuildString("'", CogDisplayName(object),
    "' attached to '", CogDisplayName(newParent), "'");

  mOldObjectLocation = object->GetHierarchyIndex();

  if (Space* space = object->GetSpace())
    mSpaceWasModified = space->GetModified();
}

//**************************************************************************************************
void AttachOperation::Undo()
{
  // Resolve handles
  Cog* object = mObject;
  Cog* parent = mParent;

  if(object == nullptr || parent == nullptr)
    return;

  // Record that the object was removed before detaching it (detaching will invalidate the child id)
  LocalModifications::GetInstance()->ChildRemoved(parent->has(Hierarchy), object);

  // Detach the object
  if(mRelativeAttach)
    object->DetachRelative();
  else
    object->Detach();

  object->PlaceInHierarchy(mOldObjectLocation);

  Space* space = object->GetSpace();
  if (space && mSpaceWasModified == false)
    space->MarkNotModified();
}

//**************************************************************************************************
void AttachOperation::Redo()
{
  // Resolve handles
  Cog* object = mObject;
  Cog* parent = mParent;
  
  if(object == nullptr || parent == nullptr)
    return;

  // Attach the object
  if(mRelativeAttach)
    object->AttachToRelative(parent);
  else
    object->AttachTo(parent);

  // Store the new child id if we haven't already
  if(mNewChildId == PolymorphicNode::cInvalidUniqueNodeId)
    mNewChildId = object->mChildId;
  // Otherwise assign the new child id we were assigned the first time we were attached
  else
    object->mChildId = mNewChildId;

  // Record that the object was attached
  LocalModifications::GetInstance()->ChildAdded(parent->has(Hierarchy), object);

  // If we attached relative, the Transform properties may have been modified. We should explicitly
  // mark them as modified to ensure the object has the correct position, rotation, and scale
  if(mRelativeAttach)
    object->MarkTransformModified();

  if (Space* space = object->GetSpace())
    space->MarkModified();
}

//--------------------------------------------------------------------------------- Detach Operation
//**************************************************************************************************
DetachOperation::DetachOperation(Cog* object, bool relativeDetach) :
  mRelativeDetach(relativeDetach),
  mObjectUndoHandle(object)
{
  Cog* parent = object->GetParent();
  ErrorIf(object == nullptr || parent == nullptr, "Invalid object given to DetachOperation");

  mName = BuildString("'", CogDisplayName(object),
    "' detached from '", CogDisplayName(parent), "'");

  mParentUndoHandle = parent;

  mOldChildId = object->mChildId;
  mOldObjectLocation = object->GetHierarchyIndex();

  // We should only store/restore modifications if we are a non-locally added child of an Archetype
  Cog* archetypeContext = object->FindNearestArchetypeContext();
  mStoreModifications = (archetypeContext && archetypeContext != object);
  
  if(mStoreModifications)
    mOldModifications.Cache(object);

  if (Space* space = object->GetSpace())
    mSpaceWasModified = space->GetModified();
}

//**************************************************************************************************
void DetachOperation::Undo()
{
  // Resolve handles
  Cog* object = mObjectUndoHandle;
  Cog* parent = mParentUndoHandle;

  if(object == nullptr || parent == nullptr)
    return;

  // Apply the old modifications to the object
  if(mStoreModifications)
  {
    // When we were detached, we may have gotten more modifications added to us that were in the
    // base Archetype. Clear those before restoring the old modifications
    LocalModifications::GetInstance()->ClearModifications(object, true, false);

    // Restore
    mOldModifications.ApplyModificationsToObject(object, false);
  }

  // Attach the object
  if(mRelativeDetach)
    object->AttachToRelative(parent);
  else
    object->AttachTo(parent);

  // Restore data
  object->mChildId = mOldChildId;
  object->PlaceInHierarchy(mOldObjectLocation);

  // Record that the object was removed
  LocalModifications::GetInstance()->ChildAdded(parent->has(Hierarchy), object);

  Space* space = object->GetSpace();
  if (space && mSpaceWasModified == false)
    space->MarkNotModified();
}

//**************************************************************************************************
void DetachOperation::Redo()
{
  // Resolve handles
  Cog* object = mObjectUndoHandle;
  Cog* parent = mParentUndoHandle;

  if(object == nullptr || parent == nullptr)
    return;

  if(mStoreModifications)
  {
    Cog* archetypeCog = object->FindNearestArchetypeContext();
    Archetype* archetype = archetypeCog->GetArchetype();
    archetype->GetAllCachedModifications().ApplyModificationsToChildObject(archetypeCog, object, true);
  }

  // Record that the object was removed before actually detaching it (this is because detaching
  // will invalidate the child id)
  LocalModifications::GetInstance()->ChildRemoved(parent->has(Hierarchy), object);

  // Detach the object
  if(mRelativeDetach)
    object->DetachRelative();
  else
    object->Detach();

  // We should mark the Transform, as modified so that the object is re-created at its current
  // position, rotation, and scale (instead of what's in the Archetype)
  object->MarkTransformModified();

  if (Space* space = object->GetSpace())
    space->MarkModified();
}

//------------------------------------------------------------ Reorder Operation
//******************************************************************************
ReorderOperation::ReorderOperation(Cog* movingObject, uint movingToIndex) : 
  mWasParentChildOrderLocallyModified(false),
  mMovingObjectHandle(movingObject)
{
  ErrorIf(movingObject == nullptr, "Invalid object given to ReorderOperation");

  mName = BuildString("Reorder object '", CogDisplayName(movingObject), "'");

  mMovingFromIndex = movingObject->GetHierarchyIndex();
  mMovingToIndex = movingToIndex;

  // If we're moving closer to the front of the list, we have to account
  // for us taking more room before the index we came from
  if(mMovingToIndex < mMovingFromIndex)
    mMovingFromIndex++;

  // Store whether or not the child order was already overridden so that
  // when we undo, we can properly restore that state
  LocalModifications* modifications = LocalModifications::GetInstance();
  Cog* parent = movingObject->GetParent();
  if(parent)
    mWasParentChildOrderLocallyModified = modifications->IsChildOrderModified(parent->has(Hierarchy));

  if(Space* space = movingObject->GetSpace())
    mSpaceWasModified = space->GetModified();
}

//******************************************************************************
void ReorderOperation::Undo()
{
  if(Cog* object = mMovingObjectHandle)
  {
    object->PlaceInHierarchy(mMovingFromIndex);

    if(Cog* parent = object->GetParent())
    {
      // Restore the overridden state
      LocalModifications* modifications = LocalModifications::GetInstance();
      modifications->SetChildOrderModified(parent->has(Hierarchy), mWasParentChildOrderLocallyModified);
    }

    Space* space = object->GetSpace();
    if (mSpaceWasModified == false)
      space->MarkNotModified();
  }
}

//******************************************************************************
void ReorderOperation::Redo()
{
  if(Cog* object = mMovingObjectHandle)
  {
    object->PlaceInHierarchy(mMovingToIndex);

    if(Cog* parent = object->GetParent())
    {
      // Restore the overridden state
      LocalModifications* modifications = LocalModifications::GetInstance();
      modifications->SetChildOrderModified(parent->has(Hierarchy), true);
    }

    if (Space* space = object->GetSpace())
      space->MarkModified();
  }
}

//----------------------------------------------------- Create Destroy Operation
//******************************************************************************
CreateDestroyOperation::CreateDestroyOperation(Cog* object, ObjectOperationMode::Enum mode)
{
  ErrorIf(object == nullptr, "Invalid object given to CreateDestroyOperation");

  String opType = (mode == ObjectOperationMode::Create) ? "Create" : "Destroy";
  mName = BuildString(opType, " '", CogDisplayName(object), "'");

  mMode = mode;

  // If the object has an Archetype, mark the translation as modified so that when we destroy it,
  // we can create it back at the same location. If the object does not have Archetype, the
  // translation will be saved out without having to mark Translation as modified (the whole
  // object will be saved)
  if(mMode == ObjectOperationMode::Create && object->GetArchetype())
    object->MarkTransformModified();

  mRestoreState.StoreObjectState(object);
}

//******************************************************************************
void CreateDestroyOperation::Undo()
{
  if(mMode == ObjectOperationMode::Destroy)
  {
    mRestoreState.RestoreObject( );

    mCanPatch = false;
  }
  else
  {
    mRestoreState.DestroyObject(true);
  }
}

//******************************************************************************
void CreateDestroyOperation::Redo()
{
  if(mMode == ObjectOperationMode::Destroy)
  {
    mCanPatch = true;
    mRestoreState.DestroyObject(false);
  }
  else
  {
    Cog* restoredObject = (Cog*)mRestoreState.RestoreObject();
    if(restoredObject)
      restoredObject->MarkTransformModified();
  }
}


//------------------------------------------------ Upload To Archetype Operation
//******************************************************************************
UploadToArchetypeOperation::UploadToArchetypeOperation(Cog* object)
{
  if(Archetype* archetype = object->GetArchetype())
    mName = String::Format("Upload To [%s] Archetype", archetype->Name.c_str());
  else
    mName = "Upload to Archetype";

  mRestoreState.StoreObjectState(object);
}

//******************************************************************************
UploadToArchetypeOperation::~UploadToArchetypeOperation()
{
  DeleteObjectsInContainer(mRebuiltCogs);
}

//******************************************************************************
void UploadToArchetypeOperation::CacheArchetype(Archetype* archetype)
{
  mCachedArchetypeData = archetype->GetStringData();
}

//******************************************************************************
void UploadToArchetypeOperation::RestoreCachedArchetype(Archetype* archetype)
{
  Space* space = mRestoreState.GetSpace();
  if(space == nullptr)
    return;

  // Load the saved data
  ObjectLoader loader;
  Status status;
  loader.OpenBuffer(status, mCachedArchetypeData);

  // Create the new object
  CogCreationContext context;
  context.mSpace = space;
  context.Source = "ArchetypeRebuild";

  Cog* object = Z::gFactory->BuildFromStream(&context, loader);

  // Initialize the object
  CogInitializer initializer(space);
  initializer.Context = nullptr;
  object->Initialize(initializer);

  initializer.AllCreated();

  object->SetArchetype(archetype);
  object->UploadToArchetype();

  // We just created the object to upload to Archetype, so we can now destroy it
  object->Destroy();

  // Rebuild all objects
  ArchetypeRebuilder::RebuildArchetypes(archetype);
}

//******************************************************************************
void UploadToArchetypeOperation::Undo()
{
  if(Cog* cog = mRestoreState.GetObject())
    RestoreCachedArchetype(cog->GetArchetype());

  // Restore the old modified object
  mRestoreState.RestoreObject();

  // Restore all rebuilt cogs
  forRange(CogRestoreState* restoreState, mRebuiltCogs.All())
    restoreState->RestoreObject();
}

//******************************************************************************
void UploadToArchetypeOperation::Redo()
{
  if(Cog* object = mRestoreState.GetObject())
  {
    // Upload the object to Archetype
    object->UploadToArchetype();

    if (Space* space = object->GetSpace())
      space->MarkModified();

    RebuildArchetypes(object);
  }
}

//******************************************************************************
void UploadToArchetypeOperation::RebuildArchetypes(Cog* cog)
{
  // Rebuild all objects. Only create restore states the first time
  if(mRebuiltCogs.Empty())
    ArchetypeRebuilder::RebuildArchetypes(cog->GetArchetype(), cog, &mRebuiltCogs);
  else
    ArchetypeRebuilder::RebuildArchetypes(cog->GetArchetype(), cog);

  if(Space* space = cog->GetSpace())
    space->ChangedObjects();
}

//-------------------------------------------- Upload To New Archetype Operation
//******************************************************************************
UploadToNewArchetypeOperation::UploadToNewArchetypeOperation(Cog* object) : 
  UploadToArchetypeOperation(object),
  mUploadedToNewArchetype(false)
{
  mNewChildId = GenerateUniqueId64();

  LocalModifications* modifications = LocalModifications::GetInstance();

  mParentWasChildOrderModified = false;
  if(Cog* parent = object->GetParent())
    mParentWasChildOrderModified = modifications->IsChildOrderModified(parent->has(Hierarchy));
}

//******************************************************************************
void UploadToNewArchetypeOperation::Undo()
{
  LocalModifications* modifications = LocalModifications::GetInstance();

  if(Cog* cog = mRestoreState.GetObject())
  {
    // Mark the new Cog as removed
    if(Cog* parent = cog->GetParent())
      modifications->ChildRemoved(parent->has(Hierarchy), cog);
  }

  // Restore the Archetype data to what it was previously.
  // If we uploaded to a new Archetype, there was no previous data to restore.
  // We could delete the newly created Archetype Resource, but for now our operation
  // do not handle resource creation / destruction
  if(!mUploadedToNewArchetype)
    RestoreCachedArchetype(mNewArchetype);

  // Restore the old modified object
  Cog* restoredCog = (Cog*)mRestoreState.RestoreObject();

  // The original child was added back
  if(restoredCog)
  {
    if(Cog* parent = restoredCog->GetParent())
    {
      Hierarchy* hierarchy = parent->has(Hierarchy);
      modifications->ChildAdded(hierarchy, restoredCog);
      modifications->SetChildOrderModified(hierarchy, mParentWasChildOrderModified);
    }
  }
}

//******************************************************************************
void UploadToNewArchetypeOperation::Redo()
{
  if(Cog* object = mRestoreState.GetObject())
  {
    // If we're uploading to a new Archetype, assign it first
    if(Archetype* newArchetype = mNewArchetype)
      object->SetArchetype(newArchetype);

    LocalModifications* modifications = LocalModifications::GetInstance();

    // Remove the old object, add the new one
    if(Cog* parent = object->GetParent())
    {
      Hierarchy* hierarchy = parent->has(Hierarchy);
      modifications->ChildRemoved(hierarchy, object);
      object->mChildId = mNewChildId;
      modifications->ChildAdded(hierarchy, object);
      modifications->SetChildOrderModified(hierarchy, true);
    }

    object->MarkTransformModified();

    // Upload the object to Archetype
    if(!mUploadedToNewArchetype)
    {
      object->UploadToArchetype();

      // Rebuild all objects
      RebuildArchetypes(object);
    }
    else
    {
      // We don't need to upload to Archetype again if this operation created it
      // because the Object is currently in the state that was uploaded to 
      // the Archetype. We only need to mark ourselves as not modified
      object->MarkNotModified();

      if(Space* space = object->GetSpace())
      {
        space->MarkModified();
        space->ChangedObjects();
      }
    }
  }
}

//------------------------------------------------ Revert To Archetype Operation
//******************************************************************************
RevertToArchetypeOperation::RevertToArchetypeOperation(Cog* object)
{
  Cog* archetypeContextCog = object->FindNearestArchetypeContext();
  Archetype* archetype = archetypeContextCog->GetArchetype();

  ErrorIf(archetype == nullptr, "Object doesn't have an Archetype");

  mName = BuildString("Reverted '", CogDisplayName(object),
    "' to '", archetype->Name, "' Archetype");

  mRestoreState.StoreObjectState(object);
}

//******************************************************************************
void RevertToArchetypeOperation::Undo()
{
  // Replace it with the restored object
  mRestoreState.RestoreObject();
}

//******************************************************************************
void RevertToArchetypeOperation::Redo()
{
  Cog* object = mRestoreState.GetObject();
  if(object == nullptr)
    return;

  // Revert to Archetype
  object->RevertToArchetype();
  if (Space* space = object->GetSpace())
    space->MarkModified();
}

//---------------------------------------------------- Clear Archetype Operation
//******************************************************************************
ClearArchetypeOperation::ClearArchetypeOperation(Cog* object)
{
  ErrorIf(object->GetArchetype() == nullptr, "Object doesn't have an Archetype");

  mName = BuildString("Clear '", object->GetArchetype()->Name,
    "' Archetype from '", CogDisplayName(object), "'");

  mRestoreState.StoreObjectState(object);
  
  mNewChildId = GenerateUniqueId64();

  LocalModifications* modifications = LocalModifications::GetInstance();

  mParentWasChildOrderModified = false;
  if(Cog* parent = object->GetParent())
    mParentWasChildOrderModified = modifications->IsChildOrderModified(parent->has(Hierarchy));
}

//******************************************************************************
void ClearArchetypeOperation::Undo()
{
  Cog* cog = mRestoreState.GetObject();
  if(cog == nullptr)
    return;

  LocalModifications* modifications = LocalModifications::GetInstance();

  Cog* parent = cog->GetParent();
  if(parent)
  {
    Hierarchy* hierarchy = parent->has(Hierarchy);
    ObjectState::ChildId childId(ZilchTypeId(Cog), mNewChildId);
    modifications->ChildRemoved(hierarchy, childId);
    modifications->SetChildOrderModified(hierarchy, mParentWasChildOrderModified);
  }

  // Replace it with the restored object
  cog = (Cog*)mRestoreState.RestoreObject();

  // If the parent is no longer modified from Archetype, the modifications
  // were already cleared (when 'NotModified' is called on Cog), so we don't
  // need to clear them here
  if(parent && parent->IsModifiedFromArchetype())
    modifications->ChildAdded(parent->has(Hierarchy), cog);

  mRestoreState.RestoreSpaceModifiedState();
}

//******************************************************************************
void ClearArchetypeOperation::Redo()
{
  Cog* cog = mRestoreState.GetObject();
  if(cog == nullptr)
    return;

  LocalModifications* modifications = LocalModifications::GetInstance();

  // We're simulating removing the old object and adding the new, so remove
  // the old id from our parent and add the new one after clearing the Archetype
  Cog* parent = cog->GetParent();
  if(parent)
  {
    Hierarchy* hierarchy = parent->has(Hierarchy);
    modifications->ChildRemoved(hierarchy, cog);
    modifications->SetChildOrderModified(hierarchy, true);
  }

  // Clear the Archetype
  cog->ClearArchetype();

  // Assign the new child id
  cog->mChildId = mNewChildId;

  if(parent)
  {
    ObjectState::ChildId childId(ZilchTypeId(Cog), mNewChildId);
    modifications->ChildAdded(parent->has(Hierarchy), childId);
  }

  // The object no longer has any modifications
  modifications->ClearModifications(cog, true, false);

  if(Space* space = cog->GetSpace())
  {
    space->ChangedObjects();
    space->MarkModified();
  }
}

}//namespace Zero
