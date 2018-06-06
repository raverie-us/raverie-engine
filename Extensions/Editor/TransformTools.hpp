///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys, Ryan Edgemon
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class DisplayScene;
class ViewportMouseEvent;

DeclareEnum3(GizmoMode, Inactive, Active, Transforming);
DeclareEnum2(GizmoGrab, Hold, Toggle);

// ManipulatorTool Events
namespace Events
{
DeclareEvent(ManipulatorToolStart);
DeclareEvent(ManipulatorToolModified);
DeclareEvent(ManipulatorToolEnd);
}

//----------------------------------------------------- ManipulatorToolEvent ---
class ManipulatorToolEvent : public ViewportMouseEvent
{
public:
  ZilchDeclareType(ManipulatorToolEvent, TypeCopyMode::ReferenceType);

  ManipulatorToolEvent(ViewportMouseEvent* event);

  OperationQueue* GetOperationQueue();
  bool GetFinished();

  HandleOf<OperationQueue> mOperationQueue;

  Location::Enum mGrabLocation;
  Rectangle mStartWorldRectangle;

  /// If marking the event as HandledEventScript == false, then modifications
  /// to 'EndWorldRectangle' will be applied internally by the ManipulatorTool.
  Rectangle mEndWorldRectangle;
};

//------------------------------------------------------------------------------

//Store data about each object being transformed
struct TransformingObject
{
  CogId ObjectId;
  Vec2 StartAreaTranslation;
  Vec3 StartWorldTranslation;
  Vec3 StartTranslation;
  Quat StartRotation;
  Vec3 StartScale;
  Vec2 StartSize;
  Vec3 StartColliderSize;

  Vec3 EndTranslation;
  Quat EndRotation;
  Vec3 EndScale;
  Vec2 EndSize;

  Rectangle StartRect;
};

//------------------------------------------------------------- Manipulator Tool
/// <Commands>
///   <command name = "Maintain Aspect Ratio">
///     <shortcut> Shift + Drag </shortcut>
///     <description>
///       While held, maintain aspect ratio of the object being manipulated.
///     </description>
///   </command>
/// </Commands>
class ManipulatorTool : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(ManipulatorTool, TypeCopyMode::ReferenceType);

  ManipulatorTool();

  /// Component Interface.
  void Serialize(Serializer& stream);
  void Initialize(CogInitializer& initializer);

  bool Active() { return mGizmoMode != GizmoMode::Inactive; }

  float GetSnapDistance();
  void SetSnapDistance(float distance);

  bool IsActiveAndHasValidSelection();
  bool CheckMouseManipulation(ViewportMouseEvent* e);

  /// Set the gizmo mode when activated / deactivated.
  void OnToolActivate(Event*);
  void OnToolDeactivate(Event*);

  // Event Response.
  void OnLeftMouseDown(ViewportMouseEvent* e);
  void OnLeftMouseUp(ViewportMouseEvent* e);
  void OnMouseMove(ViewportMouseEvent* e);

  /// Gizmo Interface.
  void TestMouseMove(ViewportMouseEvent* e);
  void OnMouseDragStart(ViewportMouseEvent* e);
  void OnMouseDragMove(ViewportMouseEvent* e);
  void OnMouseDragEnd(ViewportMouseEvent* e);

  void OnToolDraw(Event*);

  /// Snap value being transformed.
  bool mSnapping;
  float mSnapDistance;

  /// Update box collier size with area.
  bool mSizeBoxCollider;

  bool mValidSelection;
  GizmoMode::Enum mGizmoMode;
  GizmoGrab::Enum mGrabMode;

  Vec2 mMouseDragStart;

  int mSelectedPoint;
  Location::Enum mLocation;
  Aabb mActiveAabb;
  Aabb mStartAabb;

  Array<TransformingObject> mTransformingObjects;
};

}//namespace Zero
