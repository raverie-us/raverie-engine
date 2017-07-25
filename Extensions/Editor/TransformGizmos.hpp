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
DeclareEnum2(GizmoBasis, Local, World);
DeclareEnum3(GizmoPivot, Primary, Center, Average);

/// 
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
  void OnGizmoModified(GizmoUpdateEvent* e);

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

  bool GetSnapping( );
  void SetSnapping(bool snapping);

  /// If they're holding down ctrl, we want to duplicate the objects.
  void OnMouseDragStart(ViewportMouseEvent* e) override;

  /// As the gizmo is being dragged, we want to update all objects.
  void OnGizmoModified(GizmoUpdateEvent* e);

  /// With multiple objects selected, the translation could be changed
  /// when scaled.
  bool mAffectTranslation;

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

  bool GetSnapping( );
  void SetSnapping(bool snapping);

  void OnMouseDragStart(ViewportMouseEvent* e) override;

  /// As the gizmo is being dragged, we want to update all objects.
  void OnGizmoModified(RingGizmoEvent* e);

  /// With multiple objects selected, the translation could be changed
  /// when rotated.
  bool mAffectTranslation;
};


}//namespace Zero
