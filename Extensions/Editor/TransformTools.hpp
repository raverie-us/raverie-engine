///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class DisplayScene;
class ViewportMouseEvent;

DeclareEnum3(GizmoType, Translate, Rotate, Scale);
DeclareEnum3(GizmoMode, Inactive, Active, Transforming);
DeclareEnum2(GizmoGrab, Hold, Toggle);
DeclareEnum5(GizmoAxis, XAxis, YAxis, ZAxis, ViewAxis, None);

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

//Base Gizmo Transform
struct TransformBasis
{
  TransformBasis()
  {
    Translation = Vec3(0, 0, 0);
    Axis[0] = Vec3::cXAxis;
    Axis[1] = Vec3::cYAxis;
    Axis[2] = Vec3::cZAxis;
  }
  Vec3 Translation;
  Vec3 Axis[3];
};

//------------------------------------------------------------------------ Gizmo
class TransformTool : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  
  /// Constructor.
  TransformTool();

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Set the gizmo mode when activated / deactivated.
  void OnToolActivate(Event*);
  void OnToolDeactivate(Event*);

  /// Whether or not any axis is selected.
  bool IsAxisSelected() { return mSelectedAxis != GizmoAxis::None; }
  bool Active() { return mGizmoMode != GizmoMode::Inactive; }

  /// Toggle between local / world.
  void ToggleCoordinateMode();

  // Event Response.
  void OnLeftMouseDown(ViewportMouseEvent* e);
  void OnLeftMouseUp(ViewportMouseEvent* e);
  void OnMouseMove(ViewportMouseEvent* e);
  void OnKeyDown(KeyboardEvent* e);
  virtual void OnMouseDragStart(ViewportMouseEvent* e);
  void OnMouseDragEnd(Event*);

  bool GetSnapping();
  float GetSnapAngle();
  float GetSnapDistance();
  void SetSnapAngle(float angle);
  void SetSnapDistance(float distance);

  //Function implemented in derived classes.
  virtual void TestMouseMove(ViewportMouseEvent* e) = 0;

protected:
  friend class GizmoSet;

  void RecomputeTransformBasis();
  bool CheckMouseManipulation(ViewportMouseEvent* e);
  bool OnGizmo();

  /// Snap value being transformed.
  bool mSnapping;
  float mSnapAngle;
  float mSnapDistance;

  bool mValidSelection;

  /// Affect translations for scale / rotations.
  bool mAffectTranslation;

  GizmoAxis::Enum mSelectedAxis;
  GizmoType::Enum mGizmoType;
  GizmoMode::Enum mGizmoMode;
  GizmoGrab::Enum mGrabMode;
  GizmoPivot::Enum mPivot;
  GizmoBasis::Enum mBasis;

  /// Update box collier size with area.
  bool mSizeBoxCollider;

  Array<TransformingObject> mTransformingObjects;

  TransformBasis mActiveBasis;
  TransformBasis mStartingBasis;

  /// Where the Gizmo was grabbed at.
  Vec3 mGrabPoint;
  Vec3 mEyeDirection;
  uint mPlaneAxis;
};

//-------------------------------------------------------------- Translate Gizmo
class TranslateTool : public TransformTool
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  TranslateTool();

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Gizmo Interface.
  void TestMouseMove(ViewportMouseEvent* e) override;
  void OnMouseDragStart(ViewportMouseEvent* e) override;

  virtual void OnMouseDragMove(ViewportMouseEvent* e);
  virtual void OnToolDraw(Event*);

  virtual void ApplyWorldMovement(Vec3 worldMovement);

  bool mSnapToXPlane;
  bool mSnapToYPlane;
  bool mSnapToZPlane;
  bool mDuplicateOnCtrlDrag;
};

//------------------------------------------------------------- Manipulator Tool
class ManipulatorTool : public TranslateTool
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ManipulatorTool();

  /// Component Interface.
  void Serialize(Serializer& stream) override;

  /// Gizmo Interface.
  void TestMouseMove(ViewportMouseEvent* e) override;
  void OnMouseDragStart(ViewportMouseEvent* e) override;

  void OnMouseDragMove(ViewportMouseEvent* e) override;
  void OnToolDraw(Event*) override;

  Vec2 mMouseDragStart;
  int mSelectedPoint;
  Aabb mActiveAabb;
  Aabb mStartAabb;

  bool mScalingMode;
  bool mTranslationOnly;
};

}//namespace Zero
