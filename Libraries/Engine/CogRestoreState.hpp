///////////////////////////////////////////////////////////////////////////////
///
/// \file CogRestoreState.hpp
/// 
/// Authors: Joshua Claeys
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//--------------------------------------------------------- Object Restore State
/// This is responsible for 
///
/// Operations that need to be done when saving / restoring an object
/// - Place in the same spot in the hierarchy
/// - Restore any local modifications
/// - Restore all parents 'modified form Archetype' states
/// - Update undo handles of the object and all children
class CogRestoreState : public ObjectRestoreState
{
public:
  CogRestoreState();
  CogRestoreState(Cog* cog);

  /// ObjectRestoreState Interface.
  void StoreObjectState(Object* object) override;
  Object* RestoreObject() override;

  Cog* GetObject();
  Space* GetSpace();

  /// Destroys the saved object.
  void DestroyObject(bool restoreSpaceModifiedState);

  void RestoreSpaceModifiedState();

public:
  /// When we re-create the children, we need to make sure they're re-mapped
  /// back to their undo handles.
  void StoreChildUndoIds(Cog* object);

  /// We need to update the undo handles for each child object.
  void RestoreChildUndoIds(Cog* object, uint& childIndex);

  /// The space the object was in.
  HandleOf<Space> mSpace;
  bool mSpaceWasModified;

  /// The object we're saving / creating.
  UndoHandleOf<Cog> mObject;

  /// We want to re-attach ourself to our old parent.
  UndoHandleOf<Cog> mParent;

  /// Used for placing the object back in the correct location.
  uint mOldObjectIndex;

  /// All the children of the object being deleted need to have their id's
  /// remapped. They are stored in depth first order.
  Array<UndoObjectId> mChildUndoIds;

  /// We want to restore all local modifications made to each object.
  CachedModifications mCachedModifications;

  /// The object data will be stored here.
  String mSerializationBuffer;

  /// The child id will be stored when we save the object to a buffer, but in
  /// the case of detaching the object, we need to store the old child id.
  Guid mChildId;
};

}//namespace Zero
