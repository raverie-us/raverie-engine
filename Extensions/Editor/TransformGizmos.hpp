///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Ryan Edgemon
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward Declarations
class Transform;
class Orientation;
class OperationQueue;

class ViewportMouseEvent;
class GizmoUpdateEvent;
class UpdateEvent;

//----------------------------------------------------------------------- Events
namespace Events
{
  DeclareEvent(GizmoObjectsDuplicated);
}

//---------------------------------------------------------- Transforming Object
class ObjectTransformState
{
public:
  ObjectTransformState( )
  {
    StartWorldTranslation = Vec3::cZero;
    StartTranslation = Vec3::cZero;
    StartRotation = Quat::cIdentity;
    StartScale = Vec3::cZero;
    StartSize = Vec2::cZero;

    EndTranslation = Vec3::cZero;
    EndRotation = Quat::cIdentity;
    EndScale = Vec3::cZero;
    EndSize = Vec2::cZero;
  }

  Handle MetaObject;

  Vec3 StartWorldTranslation;
  Vec3 StartTranslation;
  Quat StartRotation;
  Vec3 StartScale;
  Vec2 StartSize;

  Vec3 EndTranslation;
  Quat EndRotation;
  Vec3 EndScale;
  Vec2 EndSize;

  Aabb WorldAabb;
};

//------------------------------------------------------- Object Transform Gizmo
/// Set the pivot point when a gizmo affects translation during manipulation.
/// <param name="Primary">
///   Set the pivot point as the position of the primary object in the selection.
/// </param>
/// <param name="Center">
///   Set the pivot point to be at the spacial center of the selection.
/// </param>
/// <param name="Average">
///   Set the pivot point to be at the average position of all objects in the selection.
/// </param>
DeclareEnum3(GizmoPivot, Primary, Center, Average);


class ObjectTransformGizmo : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  
  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Adds an object to be modified by this gizmo.
  void AddObject(HandleParam meta);
  void RemoveObject(HandleParam meta);
  void ClearObjects();

  /// Object getters.
  uint GetObjectCount();
  Handle GetObjectAtIndex(uint index);
  ObjectTransformState GetObjectStateAtIndex(uint index);

  /// If set, this Gizmo will add operations for all modifications to cogs.
  void SetOperationQueue(OperationQueue* opQueue);

  /// Toggle between local / world.
  void ToggleCoordinateMode();
  
  /// Store the state of every object.
  virtual void OnMouseDragStart(ViewportMouseEvent* e);

  /// As the gizmo is being dragged, we want to update all objects.
  void OnGizmoModified(GizmoUpdateEvent* e);

  /// Queue the final changes to the objects in the given operation queue.
  void OnMouseDragEnd(Event* e);

  /// Updates the position of the gizmo based on the current list of objects.
  virtual void UpdateGizmoBasis();

  /// We want to update the gizmo basis every frame to reflect any
  /// changes made to the objects we're modifying.
  void OnFrameUpdate(UpdateEvent* e);

  /// Setters / Getters.
  void SetBasis(GizmoBasis::Enum basis);
  GizmoBasis::Enum GetBasis();

  void SetPivot(GizmoPivot::Enum basis);
  GizmoPivot::Enum GetPivot();

  Transform* mTransform;

  GizmoBasis::Enum mBasis;
  GizmoPivot::Enum mPivot;

  OperationQueue* mOperationQueue;

  Array<Handle> mObjects;
  HashSet<Handle> mNewObjects;

  /// Gizmo type-specific members.
  bool mRotateGizmo;
  unsigned mDuplicateCorrectIndex;

  /// Update BoxCollider size with area.
  bool mSizeBoxCollider;

  bool mDragging;

  /// Stores the state of all objects when a manipulation starts.
  Array<ObjectTransformState> mObjectStates;
};

//---------------------------------------------------------- Cog Translate Gizmo
class ObjectTranslateGizmo : public ObjectTransformGizmo
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// If they're holding down ctrl, we want to duplicate the objects.
  void OnMouseDragStart(ViewportMouseEvent* e) override;

  /// As the gizmo is being dragged, we want to update all objects.
  void OnGizmoModified(TranslateGizmoUpdateEvent* e);

  /// Special command to place an object on the surface of another object.
  void SnapToSurface(GizmoUpdateEvent* e, Vec3* movementOut);

  Vec3 mStartPosition;

  bool mDuplicateOnCtrlDrag;
};

//-------------------------------------------------------------- Cog Scale Gizmo
class ObjectScaleGizmo : public ObjectTransformGizmo
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// If they're holding down ctrl, we want to duplicate the objects.
  void OnMouseDragStart(ViewportMouseEvent* e) override;

  /// As the gizmo is being dragged, we want to update all objects.
  void OnGizmoModified(ScaleGizmoUpdateEvent* e);

  /// With multiple objects selected, allow their spacial-offest to be affected
  /// about the chosen pivot point, while being locally scaled with 'mAffectScale'.
  bool mAffectTranslation;
  /// With multiple objects selected, allow their local scale to be affected
  /// while being spacially-offset (with 'AffectTranslation') about the chosen pivot point.
  bool mAffectScale;

  /// Used when dragging on the view axis to determine which direction 
  Vec3 mEyeDirection;
};

//------------------------------------------------------------- Cog Rotate Gizmo
class ObjectRotateGizmo : public ObjectTransformGizmo
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  void OnMouseDragStart(ViewportMouseEvent* e) override;

  /// As the gizmo is being dragged, we want to update all objects.
  void OnGizmoModified(RotateGizmoUpdateEvent* e);

  /// With multiple objects selected, allow their spacial-offest to be rotated
  /// about the chosen pivot point, while being locally rotated with 'mAffectRotation'.
  bool mAffectTranslation;
  /// With multiple objects selected, allow their local rotation to be affected
  /// while being spacially-rotated (with 'AffectTranslation') about the chosen pivot point.
  bool mAffectRotation;
};


}//namespace Zero
