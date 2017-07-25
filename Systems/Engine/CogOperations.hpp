///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorOperations.hpp
/// Declaration of the PropertyOperation class.
/// 
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "ObjectLoader.hpp"

namespace Zero
{

/// Attaches 'object' to 'parent' and queues the operation.
void AttachObject(OperationQueue* queue, Cog* object, Cog* parent, 
                  bool relative = true);

/// Detaches the given object from its parent and queues the operation.
void DetachObject(OperationQueue* queue, Cog* object, bool relative = true);

/// Moves the object to the given index in its current hierarchy list.
void MoveObjectIndex(OperationQueue* queue, Cog* objectToMove, uint index);

/// Moves the given object to the new parent at the given index in the
/// new parent's hierarchy.
void MoveObject(OperationQueue* queue, Cog* objectToMove, Cog* newParent,
                uint indexInNewParent, bool relativeAttach = true);

/// Destroys the given object and queues the operation.
void DestroyObject(OperationQueue* queue, Cog* object);

/// Queues the creation of the given object (does not create an object).
void ObjectCreated(OperationQueue* queue, Cog* object);

/// Creates an object from the given archetype and queues the operation.
Cog* CreateFromArchetype(OperationQueue* queue, Space* space, 
                         Archetype* archetype, Vec3Param translation,
                         QuatParam rotation = Quat::cIdentity, 
                         Vec3Param scale = Vec3(0,0,0));

/// Creates an object from the given archetype and queues the operation.
Cog* CreateFromArchetype(OperationQueue* queue, Space* space, 
                         StringParam source, Vec3Param translation,
                         QuatParam rotation = Quat::cIdentity, 
                         Vec3Param scale = Vec3(0,0,0));

/// Uploads the modifications of the given Cog to its current Archetype.
void UploadToArchetype(OperationQueue* queue, Cog* cog);

/// Returns a new Archetype if it was created
Archetype* UploadToArchetype(OperationQueue* queue, Cog* cog, StringParam archetypeName,
                             Archetype* baseArchetype = nullptr);

void RevertToArchetype(OperationQueue* queue, Cog* cog);
void ClearArchetype(OperationQueue* queue, Cog* cog);

//---------------------------------------------- Create From Archetype Operation
class CreateFromArchetypeOperation : public Operation
{
public:
  /// Constructor.
  CreateFromArchetypeOperation(Space* space, Archetype* archetype,
                               Vec3Param location, QuatParam rotation,
                               Vec3Param scale);

  /// Operation Interface.
  void Undo() override;
  void Redo() override;
  
  /// Create the object.
  Cog* DoCreation();

  HandleOf<Archetype> mArchetype;
  UndoHandleOf<Cog> mUndoHandle;
  Array<UndoObjectId> mComponentHandles;

  HandleOf<Space> mSpace;
  bool mSpaceWasModified;
  Vec3 mLocation;
  Quat mRotation;
  // If scale is zero do not override scale
  Vec3 mScale;
};

//--------------------------------------------------------------------------------- Attach Operation
class AttachOperation : public Operation
{
public:
  /// Constructor.
  AttachOperation(Cog* object, Cog* newParent, bool relativeAttach);

  /// Operation Interface.
  void Undo() override;
  void Redo() override;

  /// Object handles.
  UndoHandleOf<Cog> mObject;
  UndoHandleOf<Cog> mParent;

  /// When we're first attached to the new object, it will assign us a child id. If undone and
  /// redone, we want to maintain that new child id as operations after this may rely on that.
  Guid mNewChildId;

  /// The Space hierarchy location the object was at before the operation.
  uint mOldObjectLocation;

  bool mRelativeAttach;
  bool mSpaceWasModified;
};

//--------------------------------------------------------------------------------- Detach Operation
class DetachOperation : public Operation
{
public:
  /// Constructor.
  DetachOperation(Cog* object, bool relativeDetach);

  /// Operation Interface.
  void Undo() override;
  void Redo() override;

  /// Object handles.
  UndoHandleOf<Cog> mObjectUndoHandle;
  UndoHandleOf<Cog> mParentUndoHandle;

  /// Our modifications will only be affected if we are a non-locally added child of an Archetype.
  bool mStoreModifications;

  /// When we detach ourself, our local modifications will change. When we undo that, we want to
  /// restore those modifications.
  CachedModifications mOldModifications;

  /// When the object is re-attached, we want it to have its old child id.
  Guid mOldChildId;

  /// The hierarchy location the object was at before the operation.
  uint mOldObjectLocation;

  bool mRelativeDetach;
  bool mSpaceWasModified;
};

//------------------------------------------------------------ Reorder Operation
// This operation assumes the objects are already siblings
class ReorderOperation : public Operation
{
public:
  /// Constructor.
  ReorderOperation(Cog* movingObject, uint movingToIndex);

  /// Operation Interface.
  void Undo() override;
  void Redo() override;

  UndoHandleOf<Cog> mMovingObjectHandle;
  uint mMovingFromIndex;
  uint mMovingToIndex;
  bool mWasParentChildOrderLocallyModified;
  bool mSpaceWasModified;
};

//----------------------------------------------------- Create Destroy Operation
DeclareEnum2(ObjectOperationMode, Create, Destroy);

class CreateDestroyOperation : public Operation
{
public:
  // Required for 'ConnectThisTo' macro
  typedef CreateDestroyOperation ZilchSelf;

  /// Constructor.
  CreateDestroyOperation(Cog* object, ObjectOperationMode::Enum mode);

  /// Operation Interface.
  void Undo() override;
  void Redo() override;

  ObjectOperationMode::Enum mMode;
  CogRestoreState mRestoreState;
};

//------------------------------------------------ Upload To Archetype Operation
class UploadToArchetypeOperation : public Operation
{
public:
  /// Constructor.
  UploadToArchetypeOperation(Cog* object);
  ~UploadToArchetypeOperation();

  void CacheArchetype(Archetype* archetype);
  void RestoreCachedArchetype(Archetype* archetype);

  /// Operation Interface.
  void Undo() override;
  void Redo() override;

  void RebuildArchetypes(Cog* cog);

  String mCachedArchetypeData;
  CogRestoreState mRestoreState;

  /// When we upload to Archetype, we have to rebuild all live Cogs assigned to that Archetype.
  /// There is a chance for data loss here. Example:
  ///   - You have multiple objects with the Enemy Archetype in the level
  ///   - One instance locally modifies the child Gun Archetype
  ///   - On another instance, remove the Gun and upload to Archetype
  ///   - When the object is uploaded, the instance with the modified gun will no longer have a gun
  ///   - When saving that object, the data that described the modification to the gun is lost
  ///   - Undoing the Archetype upload will not bring that data back
  /// Because of this, we need to store the original modified objects state before uploading to
  /// Archetype so that we can avoid losing data.
  Array<CogRestoreState*> mRebuiltCogs;
};

//-------------------------------------------- Upload To New Archetype Operation
class UploadToNewArchetypeOperation : public UploadToArchetypeOperation
{
public:
  /// Constructor.
  UploadToNewArchetypeOperation(Cog* object);

  /// Operation Interface.
  void Undo() override;
  void Redo() override;
  
  /// If the Archetype was newly created.
  bool mUploadedToNewArchetype;

  /// This operation will mark the parent Hierarchy as child order modified, so
  /// we need to store this to properly restore the old state.
  bool mParentWasChildOrderModified;

  /// The Archetype assigned to the Cog after the operation is done.
  HandleOf<Archetype> mNewArchetype;

  /// This operation will remove the old Archetyped object, and add the new one.
  /// We need a newly generated child id to assign the new object, and we want it
  /// to be consistent every time this operation is redone.
  Guid mNewChildId;
};

//------------------------------------------------ Revert To Archetype Operation
class RevertToArchetypeOperation : public Operation
{
public:
  /// Constructor.
  RevertToArchetypeOperation(Cog* object);

  /// Operation Interface.
  void Undo() override;
  void Redo() override;

  CogRestoreState mRestoreState;
};

//---------------------------------------------------- Clear Archetype Operation
/// Clearing an Archetype will act like the old object was removed and a new
/// one was created. This is because trying to patch an Archetyped object with
/// a non-Archetyped object (after clear) doesn't make any sense.
class ClearArchetypeOperation : public Operation
{
public:
  /// Constructor.
  ClearArchetypeOperation(Cog* object);

  /// Operation Interface.
  void Undo() override;
  void Redo() override;

  /// This operation will remove the old Archetyped object, and add the cleared object.
  /// We need a newly generated child id to assign the new object, and we want it
  /// to be consistent every time this operation is redone.
  u64 mNewChildId;

  /// This operation will mark the parent Hierarchy as child order modified, so
  /// we need to store this to properly restore the old state.
  bool mParentWasChildOrderModified;

  CogRestoreState mRestoreState;
};

}//namespace Zero
