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

//------------------------------------------------------------------------------

//Store data about each object being transformed
struct TransformingObject
{
  Handle ObjectMeta;

  CogId ObjectId;
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

//------------------------------------------------------------- Manipulator Tool
class ManipulatorTool : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ManipulatorTool();

  /// Component Interface.
  void Serialize(Serializer& stream);
  void Initialize(CogInitializer& initializer);

  bool Active() { return mGizmoMode != GizmoMode::Inactive; }

  float GetSnapDistance();
  void SetSnapDistance(float distance);

  bool OnGizmo();
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
  void OnMouseDragEnd(Event*);

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
  Aabb mActiveAabb;
  Aabb mStartAabb;

  Array<TransformingObject> mTransformingObjects;
};

}//namespace Zero
