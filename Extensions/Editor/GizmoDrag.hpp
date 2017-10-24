///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward Declarations
class Editor;
class MouseCapture;
class ViewportMouseEvent;
class MouseEvent;

//----------------------------------------------------------------------- Events
namespace Events
{
  /// Sent out right before the GizmoDrag Component starts a drag.
  DeclareEvent(GizmoPreDrag);
}

//----------------------------------------------------------- Gizmo Update Event
class GizmoUpdateEvent : public GizmoEvent
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructors.
  GizmoUpdateEvent(Cog* gizmoCog, ViewportMouseEvent* e);
  GizmoUpdateEvent(GizmoUpdateEvent* rhs);

  /// Movement of the mouse (in world space) constrained against a gizmo's drag-mode.
  Vec3 mConstrainedWorldMovement;

  /// Difference of the mouse position (in world space) constrained against a
  /// gizmo's drag-mode with that position last frame.
  Vec3 mConstrainedWorldDelta;

  /// Initial mouse-click position (in world space) constrained against a
  /// gizmo's drag-mode.
  Vec3 mInitialGrabPoint;
};

//------------------------------------------------- Translate Gizmo Update Event
class TranslateGizmoUpdateEvent : public GizmoUpdateEvent
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  TranslateGizmoUpdateEvent(GizmoUpdateEvent* e);

  /// Mouse movement that has undergone alterations, ex: Snapping.
  Vec3 mProcessedMovement;
};

//----------------------------------------------------- Scale Gizmo Update Event
class ScaleGizmoUpdateEvent : public GizmoUpdateEvent
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  ScaleGizmoUpdateEvent(GizmoUpdateEvent* e);

  /// Mouse movement that has undergone transformation to compute a change in
  /// scale.  May or may not include snapping.
  Vec3 mProcessedScale;
};

//---------------------------------------------------- Rotate Gizmo Update Event
class RotateGizmoUpdateEvent : public GizmoUpdateEvent
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  RotateGizmoUpdateEvent(GizmoUpdateEvent* e);

  /// RingGizmo rotation that has undergone alterations, ex: Snapping.
  float mProcessedRotation;

  /// Normalized axis of movement.
  Vec3 mSelectedAxis;
};
//------------------------------------------------------------------- Gizmo Drag
DeclareEnum3(GizmoDragMode, Line, Plane, ViewPlane);
DeclareEnum2(GizmoGrabMode, Hold, Toggle);

class GizmoDrag : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Starts the drag
  void StartDrag(ViewportMouseEvent* e);

  /// Returns the most visible plane normal from the cameras direction.
  /// Used to choose the best plane (the most visible or biggest absolute dot
  /// product with the eye) for movement projection.
  Vec3 GetLineDragPlane(Vec3Param dragDirection, Vec3Param eyeDirection);

  /// The point where the mouse was when the drag started.
  Vec3 GetGrabPoint();

  /// Returns whether or not a drag is currently active.
  bool GetDragActive();

//private:
  /// Used to detect our own mouse drags.
  void OnLeftMouseDown(ViewportMouseEvent* e);
  void OnLeftMouseUp(ViewportMouseEvent* e);
  void OnMouseMove(ViewportMouseEvent* e);
  void OnLeftMouseDrag(MouseEvent* e);

  /// Reset mMouseDown.
  void OnMouseExitGizmo(Event*);

  /// Update the gizmo as we move.
  void OnMouseDragMove(ViewportMouseEvent* e);
  /// We need to set the drag active flag when the drag ends.
  void OnMouseDragEnd(Event*);

  /// Casts the given ray against the current drag plane. Not valid to call
  /// if we aren't currently dragging.
  Vec3 CastRayAgainstDragPlane(const Ray& worldRay);

  /// Dependencies.
  Transform* mTransform;
  MouseCapture* mMouseCapture;

  /// If set, the gizmo will automatically start a drag once the mouse goes down
  /// and moves the specified distance (mDragDistance).
  bool mAutoDrag;

  /// Whether or not the mouse is currently down on this Gizmo.
  bool mMouseDown;

  /// The view-port position that the mouse went down on. 
  Vec2 mMouseDownPosition;

  /// How far the mouse needs to move before starting a drag.
  float mDragDistance;

  /// The plane we're testing the ray against
  Plane mDragPlane;

  /// The world translation of where the mouse clicked at the start of a drag.
  Vec3 mInitialGrabPoint;

  /// Previous frame's mouse world position
  Vec3 mPreviousMouseWorldPosition;

  /// The eye direction of the camera when the manipulation started.
  Vec3 mEyeDirection;

  /// Determine if dragging occurs in a line or on a plane.
  GizmoDragMode::Enum mDragMode;
  /// Determine if mouse needs to be held or clicked for dragging to occur.
  GizmoGrabMode::Enum mGrabMode;
  /// Direction drag occurs.
  Vec3 mLineDirection;
  /// Plane on which drag occurs, depending on DragMode. 
  Vec3 mPlaneNormal;
  bool mNormalInWorld;
};

}//namespace Zero
