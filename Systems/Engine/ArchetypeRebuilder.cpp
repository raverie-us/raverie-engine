////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// \file ArchetypeRebuilder.cpp
///
/// Authors: Joshua Claeys
/// Copyright 2010-2016, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "ObjectSaver.hpp"
#include "CogRestoreState.hpp"

namespace Zero
{

//-------------------------------------------------------------------------------- Cog Replace Event
namespace Events
{
DefineEvent(CogReplaced);
}

ZilchDefineType(CogReplaceEvent, builder, type)
{
}

//**************************************************************************************************
CogReplaceEvent::CogReplaceEvent(Cog* oldCog, Cog* newCog) :
  mOldCog(oldCog),
  mNewCog(newCog)
{
  mOldIndex = oldCog->GetId().ToUint64();
  mNewIndex = newCog->GetId().ToUint64();
}

//------------------------------------------------------------------------------ Archetype Rebuilder
void ReplaceInSelection(Cog* oldCog, Cog* newCog,
                        HashSet<MetaSelection*>* modifiedSelections);

//**************************************************************************************************
void ArchetypeRebuilder::RebuildArchetypes(Archetype* modifiedArchetype, Cog* ignore,
                                           Array<CogRestoreState*>* restoreStates)
{
  if(modifiedArchetype == nullptr)
    return;

  PushErrorContextObject("Rebuild Archetype", modifiedArchetype);

  // Gather all objects that need to be reloaded in all Game Sessions
  // This is so iteration can be safe later
  HashSet<Cog*> objectsToReload;

  forRange(GameSession* gameSession, Z::gEngine->GetGameSessions())
  {
    forRange(Space* space, gameSession->GetAllSpaces())
    {
      forRange(Cog& cog, space->mCogList.All())
      {
        // We don't want to create an object that's going to be destroyed
        if(cog.GetMarkedForDestruction() || &cog == ignore)
          continue;

        // Walk all base types to see if the current cog's Archetype derives from the
        // Archetype we're rebuilding
        Resource* currArchetype = cog.mArchetype;
        while(currArchetype)
        {
          if(currArchetype == modifiedArchetype)
          {
            // If the Archetype is contained under another Archetype, we want to rebuild it from
            // the highest Archetype context. This way, when we delete and re-create it, all
            // potential modifications on the root Archetype will be properly applied to this
            // object we want to rebuild.
            // An alternative to this solution is applying cached modifications (stored on
            // Archetype) to the object, save it out, then rebuild it. This method is already
            // used when uploading to Archetype on a child of another Archetype.
            // See Cog::UploadToArchetype.
            objectsToReload.Insert(cog.FindNearestArchetypeContext());
            break;
          }

          currArchetype = currArchetype->GetBaseResource();
        }
      }
    }
  }

  // If there are no live objects with that Archetype, no need to do anything
  if(objectsToReload.Empty())
    return;

  ZPrint("Rebuilding %d Archetypes of '%s'\n", objectsToReload.Size(), modifiedArchetype->Name.c_str());

  HashSet<MetaSelection*> modifiedSelections;

  // Replace all needed objects
  forRange(Cog* oldCog, objectsToReload.All())
  {
    // Store the state of the Cog for undo/redo before rebuilding it
    if(restoreStates)
    {
      CogRestoreState* restoreState = new CogRestoreState();
      restoreState->StoreObjectState(oldCog);
      restoreStates->PushBack(restoreState);
    }

    RebuildCog(oldCog, &modifiedSelections);
  }

  forRange(MetaSelection* selection, modifiedSelections.All())
    selection->FinalSelectionUpdated();
}

//**************************************************************************************************
Cog* ArchetypeRebuilder::RebuildCog(Cog* cog)
{
  HashSet<MetaSelection*> modifiedSelections;

  // Rebuild the object
  Cog* newCog = ArchetypeRebuilder::RebuildCog(cog, &modifiedSelections);

  // Notify that the selections have been changed
  forRange(MetaSelection* selection, modifiedSelections.All())
    selection->FinalSelectionUpdated();

  return newCog;
}

//**************************************************************************************************
void RestoreUndoHandles(Cog* oldCog, Cog* newCog)
{
  Z::gUndoMap->UpdateHandleIfExists(oldCog, newCog);

  // Update Component handles
  forRange(Component* newComponent, newCog->GetComponents())
  {
    BoundType* componentType = ZilchVirtualTypeId(newComponent);
    if(Component* oldComponent = oldCog->QueryComponentType(componentType))
      Z::gUndoMap->UpdateHandleIfExists(oldComponent, newComponent);
  }

  // Update child Objects
  forRange(Cog& newChild, newCog->GetChildren())
  {
    if(Cog* oldChild = oldCog->FindChildByChildId(newChild.mChildId))
      RestoreUndoHandles(oldChild, &newChild);
  }
}

//**************************************************************************************************
Cog* ArchetypeRebuilder::RebuildCog(Cog* oldCog, HashSet<MetaSelection*>* modifiedSelections)
{
  // TODO: For now, don't rebuild objects that are transient. We could mark them as non-transient,
  // rebuild, then mark the new one as transient
  if (oldCog->GetTransient())
    return nullptr;

  ErrorIf(oldCog->FindNearestArchetypeContext() != oldCog, "Can only rebuild root contexts.");

  LocalModifications* modifications = LocalModifications::GetInstance();
  Space* space = oldCog->GetSpace();

  // To rebuild, we're going to save the object to a string then re-load it
  // from that string. This way it retains any local modifications

  // TODO: There is a possible issue with this action:
  // One example is regarding Gizmos. The Translate tool creates a Translate Gizmo and
  // destroys it when the tool is de-activated. If we rebuild that Gizmo due to an Archetype change,
  // the Tool's handle to it will now be null, and the Gizmo will forever be sitting there.
  // We should consider creating the new Cog with the old Cog's id.

  // Save the object
  ObjectSaver saver;
  saver.OpenBuffer();
  saver.SaveInstance(oldCog);

  Guid childId = oldCog->mChildId;

  // Load the saved data
  ObjectLoader loader;
  Status status;
  loader.OpenBuffer(status, saver.GetString());

  // Create the new object
  CogCreationContext context;
  context.mSpace = space;
  context.Source = "ArchetypeRebuild";
  Cog* updatedCog = Z::gFactory->BuildFromStream(&context, loader);

  if(updatedCog == nullptr)
  {
    Error("Failed to rebuild object from Archetype.");
    return nullptr;
  }

  // Initialize the new object
  CogInitializer initializer(space, oldCog->GetGameSession());
  initializer.Context = &context;
  updatedCog->Initialize(initializer);
  initializer.AllCreated();

  // Restore the child id
  updatedCog->mChildId = childId;

  // If the old object was selected, we want to re-insert the new object
  // in the old one's place
  ReplaceInSelection(oldCog, updatedCog, modifiedSelections);

  // We need to update undo handles so that the object can continue to be
  // referenced in undo / redo
  RestoreUndoHandles(oldCog, updatedCog);

  // Send an event to signal that the cog has been replaced
  CogReplaceEvent eventToSend(oldCog, updatedCog);
  space->DispatchEvent(Events::CogReplaced, &eventToSend);

  // Move to the same place on the tree
  Cog* parent = oldCog->GetParent();
  if(parent == nullptr)
  {
    // Move in root
    space->mRoots.Erase(updatedCog);
    space->mRoots.InsertAfter(oldCog, updatedCog);
  }
  else
  {
    // Replace the old child in the hierarchy
    parent->ReplaceChild(oldCog, updatedCog);
  }

  // The old object is no longer needed
  oldCog->Destroy();

  return updatedCog;
}

//**************************************************************************************************
void ReplaceInSelection(Cog* oldCog, Cog* newCog,
                        HashSet<MetaSelection*>* modifiedSelections)
{
  // If the old object was selected, we want to re-insert the new object
  // in the old one's place
  forRange(MetaSelection* selection, MetaSelection::GetAllSelections())
  {
    if(selection->Contains(oldCog))
    {
      selection->Replace(oldCog, newCog);
      if(modifiedSelections)
        modifiedSelections->Insert(selection);
      else
        selection->FinalSelectionChanged();
    }
  }

  // Recurse down through the children
  forRange(Cog& newChild, newCog->GetChildren())
  {
    // Find the old child that has the same child id
    Cog* oldChild = oldCog->FindChildByChildId(newChild.mChildId);
    if(oldChild)
      ReplaceInSelection(oldChild, &newChild, modifiedSelections);
  }
}

}//namespace Zero
