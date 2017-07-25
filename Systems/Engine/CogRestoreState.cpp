///////////////////////////////////////////////////////////////////////////////
///
/// \file CogRestoreState.cpp
/// 
/// Authors: Joshua Claeys
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//--------------------------------------------------------- Object Restore State
//******************************************************************************
CogRestoreState::CogRestoreState()
{

}

//******************************************************************************
CogRestoreState::CogRestoreState(Cog* cog)
{
  StoreObjectState(cog);
}

//******************************************************************************
void CogRestoreState::StoreObjectState(Object* object)
{
  Cog* cog = Type::DynamicCast<Cog*>(object);

  ReturnIf(cog == nullptr, , "Expected Cog");

  Space* space = cog->GetSpace();
  mSpaceWasModified = false;
  if(space)
    mSpaceWasModified = space->GetModified();

  mSpaceWasModified = true; // Temporary until we fix issues with how this works

  // Store the object
  mObject = cog;
  mOldObjectIndex = cog->GetHierarchyIndex();
  mChildId = cog->mChildId;
  mSpace = space;

  StoreChildUndoIds(cog);

  // Store the parent if it exists
  if(Cog* parent = cog->GetParent())
    mParent = parent;

  // If this object is an Archetype and a child of another Archetype, it may have modifications
  // in that Archetype required to create it in the proper state. Apply those modifications
  // to the object before saving it. These will be cleared later
  bool appliedArchetypeModifications = CogSerialization::PreProcessForCopy(cog, mCachedModifications);

  // Create a copy of our local modifications to restore later if pre-process didn't
  if(appliedArchetypeModifications == false)
    mCachedModifications.Cache(object);

  // Serialize the object to our buffer
  ObjectSaver saver;
  saver.OpenBuffer();
  saver.SaveInstance(cog);

  // Restore original modifications. This step is extra work when we plan on deleting the object
  // and could be optimized out
  if(appliedArchetypeModifications)
    CogSerialization::PostProcessAfterCopy(cog, mCachedModifications);

  mSerializationBuffer = saver.GetString();
}

//******************************************************************************
Object* CogRestoreState::RestoreObject()
{
  // Can't do anything without the space
  Space* space = mSpace;
  if(space == NULL)
    return nullptr;

  CogInitializer initializer(space);

  // If the object is still alive, we can assume it was detached
  Cog* object = mObject;
  Cog* oldObject = nullptr;
  if(object)
  {
    // Ignore it if it was being destroyed
    if(object->GetMarkedForDestruction())
      object = nullptr;

    // Destroy the old object
    oldObject = object;
    object = nullptr;

    if(oldObject)
      oldObject->Destroy();
  }

  if(object == nullptr)
  {
    ErrorIf(mSerializationBuffer.Empty(), "Invalid data buffer.");

    // Create the object from our saved buffer
    Status status;
    ObjectLoader loader;
    loader.OpenBuffer(status, mSerializationBuffer);

    CogCreationContext context;
    context.mSpace = space;
    context.Source = "Undo";

    object = Z::gFactory->BuildFromStream(&context, loader);

    // Initialize the object
    initializer.Context = NULL;
    object->Initialize(initializer);

    // Update the undo/redo handles so it can continue to be referenced
    mObject.UpdateObject(object);
    uint childIndex = 0;
    RestoreChildUndoIds(object, childIndex);
  }

  // Restore old child id
  object->mChildId = mChildId;

  LocalModifications* modifications = LocalModifications::GetInstance();

  if(Cog* parent = mParent)
  {
    // Attach to the old parent
    object->AttachTo(parent);

    // The parent object needs to know that we were locally added
    if(oldObject == nullptr)
      modifications->ChildAdded(parent->has(Hierarchy), object);
  }

  // We want the object to be in the same location in the hierarchy
  object->PlaceInHierarchy(mOldObjectIndex);

  // If the object wasn't newly created, this will do nothing
  initializer.AllCreated();

  // Restore all local modifications to the object and children
  CogSerialization::PostProcessAfterCopy(object, mCachedModifications);

  RestoreSpaceModifiedState();

  space->ChangedObjects();

  // If the old object was selected, we want to re-insert the new object
  // in the old one's place
  if(oldObject)
  {
    MetaSelection::ReplaceInAllSelections(oldObject, object);

    // Send an event to signal that the cog has been replaced
    CogReplaceEvent eventToSend(oldObject, object);
    space->DispatchEvent(Events::CogReplaced, &eventToSend);
  }

  // Update all selections
  forRange(MetaSelection* selection, MetaSelection::GetAllSelections())
  {
    if(selection->Contains(oldObject))
    {
      selection->Replace(oldObject, object);
      selection->FinalSelectionChanged();
    }
  }

  return object;
}

//******************************************************************************
Cog* CogRestoreState::GetObject()
{
  return mObject;
}

//******************************************************************************
Space* CogRestoreState::GetSpace()
{
  return mSpace;
}

//******************************************************************************
void CogRestoreState::DestroyObject(bool restoreSpaceModifiedState)
{
  Cog* object = mObject;
  Space* space = mSpace;

  if(object == nullptr || space == nullptr)
    return;

  // Mark our parent as modified
  if(Cog* parent = object->GetParent())
  {
    // Tell the parent that its child was removed
    LocalModifications* modifications = LocalModifications::GetInstance();
    modifications->ChildRemoved(parent->has(Hierarchy), object);
  }
  
  object->Destroy();

  if (restoreSpaceModifiedState)
  {
    if (mSpaceWasModified)
      space->MarkModified();
    else
      space->MarkNotModified();
  }
  else
  {
    space->MarkModified();
  }

  space->ChangedObjects();

  // Notify selections that our object has been destroyed
  MetaSelection::RemoveObjectFromAllSelections(object);
}

//******************************************************************************
void CogRestoreState::RestoreSpaceModifiedState()
{
  if(Space* space = mSpace)
  {
    if (mSpaceWasModified)
      space->MarkModified();
    else
      space->MarkNotModified();
  }
}

//******************************************************************************
void CogRestoreState::StoreChildUndoIds(Cog* object)
{
  // Store all Components
  forRange(Component* component, object->GetComponents())
  {
    UndoObjectId componentUndoId = Z::gUndoMap->GetUndoId(component);
    mChildUndoIds.PushBack(componentUndoId);
  }

  // Store all children
  forRange(Cog& child, object->GetChildren())
  {
    // Store the object
    UndoObjectId undoId = Z::gUndoMap->GetUndoId(child);
    mChildUndoIds.PushBack(undoId);

    StoreChildUndoIds(&child);
  }
}

//******************************************************************************
void CogRestoreState::RestoreChildUndoIds(Cog* object, uint& childIndex)
{
  // Update the components
  forRange(Component* component, object->GetComponents())
  {
    ReturnIf(childIndex >= mChildUndoIds.Size(), , "Invalid index in child Id's");

    // Update the undo handle
    UndoObjectId& componentUndoId = mChildUndoIds[childIndex];
    Z::gUndoMap->UpdateUndoId(componentUndoId, component);

    ++childIndex;
  }

  // Update all children
  forRange(Cog& child, object->GetChildren())
  {
    ReturnIf(childIndex >= mChildUndoIds.Size(), , "Invalid index in child Id's");

    // Update the object
    UndoObjectId& undoId = mChildUndoIds[childIndex];
    Z::gUndoMap->UpdateUndoId(undoId, child);

    ++childIndex;

    RestoreChildUndoIds(&child, childIndex);
  }
}

}//namespace Zero
