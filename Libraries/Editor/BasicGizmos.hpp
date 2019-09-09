// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

// Forward Declarations
class Transform;
class Orientation;
class GizmoDrag;
class ViewportMouseEvent;
class GizmoUpdateEvent;
class ObjectTransformState;

namespace Events
{
DeclareEvent(RingGizmoModified);
DeclareEvent(TranslateGizmoModified);
DeclareEvent(ScaleGizmoModified);
DeclareEvent(RotateGizmoModified);
} // namespace Events

class TranslateGizmoUpdateEvent : public GizmoUpdateEvent
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TranslateGizmoUpdateEvent, TypeCopyMode::ReferenceType);

  /// Constructor.
  TranslateGizmoUpdateEvent(GizmoUpdateEvent* e);

  /// Output of the 'TranslateGizmo'.
  Vec3 mGizmoWorldTranslation;
};

class ScaleGizmoUpdateEvent : public GizmoUpdateEvent
{
public:
  /// Meta Initialization.
  ZilchDeclareType(ScaleGizmoUpdateEvent, TypeCopyMode::ReferenceType);

  /// Constructor.
  ScaleGizmoUpdateEvent(GizmoUpdateEvent* e);

  /// Output of the 'ScaleGizmo'.
  Vec3 mGizmoWorldScale;
};

class RotateGizmoUpdateEvent : public GizmoUpdateEvent
{
public:
  /// Meta Initialization.
  ZilchDeclareType(RotateGizmoUpdateEvent, TypeCopyMode::ReferenceType);

  /// Constructor.
  RotateGizmoUpdateEvent(GizmoUpdateEvent* e);

  /// Output of the 'RotateGizmo'.
  float mGizmoRotation;

  /// Normalized axis about which 'FinalRotation' is applied.
  Vec3 mGizmoWorldRotationAxis;
};

namespace GizmoHelpers
{

float GetViewScale(Camera* camera, Vec3Param location);

int GetDragAxis(Vec3Param movement);
Vec3 SingleAxisToOffAxesScale(Vec3Param movement);
Vec3 SingleAxisToOffAxesScale(int dragAxis, Vec3Param movement);
Vec3 GetMovementDirection(Vec3Param movement, Mat3Param bases);

Vec3 MovementToUniformSignedLocalScale(float scaleDirection, Vec3Param worldMovement, QuatParam worldToLocal);
Vec3 MovementToUniformSignedLocalScale(Vec3Param scaleDirection, Vec3Param worldMovement, QuatParam worldToLocal);

float ProcessScale(float movement, float startDis, float starting);
Vec3 ScaleVector(Vec3Param delta, float distance, Vec3Param start);

} // namespace GizmoHelpers

/// Different snapping behaviors to suit specific needs [ex: Tile-based games].
/// Note: When group-selected, objects will snap individually.
/// <param name="Relative">
///   Will always snap in increments relative to an object's start position.
/// </param>
/// <param name="WorldAxes">
///   Will snap to world positions defined by 'SnapDistance' increments,
///   but only on non-zero drag-axis(axes) values.
/// </param>
/// <param name="WorldGrid">
///   Will first snap all 3 axes to a world position defined by 'SnapDistance'
///   increments. Then will continue to snap to world positions, but only on
///   non-zero drag-axis(axes) values.
/// </param>
DeclareEnum3(GizmoSnapMode, Relative, WorldAxes, WorldGrid);

namespace GizmoSnapping
{

Vec3 GetSnappedPosition(Vec3Param currentPosition,
                        Vec3Param worldMovement,
                        QuatParam basis,
                        GizmoDragMode::Enum dragMode,
                        GizmoSnapMode::Enum snapMode,
                        float snapDistance);

Vec3 GetSnappedScale(Vec3Param startPosition,
                     Vec3Param mouseWorldMovement,
                     GizmoSnapMode::Enum snapMode,
                     float snapDistance,
                     GizmoDragMode::Enum dragMode);

Vec3 GetSnappedVectorWorldAxes(Vec3Param start, Vec3Param end, float snapDistance);

} // namespace GizmoSnapping

/// Set the bases of operation for a gizmo to use when being manipulated.
/// <param name="Local">
///   Set bases of a gizmo to inherit from an object selection.
/// </param>
/// <param name="World">
///   Set the bases to match the standard world xyz bases.
/// </param>
DeclareEnum2(GizmoBasis, Local, World);

class SimpleGizmoBase : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(SimpleGizmoBase, TypeCopyMode::ReferenceType);

  /// Constructor.
  SimpleGizmoBase();

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  ByteColor GetColor();

  /// Dependencies.
  Transform* mTransform;
  Gizmo* mGizmo;

  /// Whether or not this Gizmo should receive mouse input.
  bool mMouseInput;

  /// Used to manually determine which Gizmo gets selected when the mouse is
  /// over multiple Gizmos. Higher priority will get picked first.
  int mPickingPriority;

  /// If set to true, the priority only applies to the hierarchy if this Gizmo.
  bool mLocalPriority;

  /// When the mouse is over the gizmo, this will be the distance from the
  /// camera to where the mouse's ray intersects with the gizmo.
  float mPickDistance;

  /// The world position that was grabbed to start the gizmo manipulation.
  Vec3 mGrabPoint;

  /// Display colors.
  Vec4 mColor, mHoverColor;

  /// If enabled, the size of the gizmo will stay the same regardless of how
  /// far away the camera is.
  bool mViewScaled;
  /// If enabled, the gizmo will scale around it's parent
  bool mUseParentAsViewScaleOrigin;

  /// Whether or not to draw on top of all objects regardless of depth.
  bool mDrawOnTop;
};

class SquareGizmo : public SimpleGizmoBase
{
public:
  /// Meta Initialization.
  ZilchDeclareType(SquareGizmo, TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Test if the mouse is over this gizmo.
  void OnGizmoRayTest(GizmoRayTestEvent* e);

  /// Draw the gizmo every frame.
  void OnFrameUpdate(Event*);

  /// The size of the square.
  Vec3 mSize;
  float mSnapDistance;

  bool mFilled;
  bool mBordered;
  bool mViewAligned;
};

DeclareEnum2(ArrowHeadType, Arrow, Cube);

class ArrowGizmo : public SimpleGizmoBase
{
public:
  /// Meta Initialization.
  ZilchDeclareType(ArrowGizmo, TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Test if the mouse is over this gizmo.
  void OnGizmoRayTest(GizmoRayTestEvent* e);

  /// We want to update the drag direction based on our orientation.
  void OnGizmoPreDrag(GizmoEvent* e);

  /// Draw the gizmo every frame.
  void OnFrameUpdate(Event*);

  /// Used to define the drag axis.
  Orientation* mOrientation;

  /// Determines if an arrow head is present at both ends of the Gizmo.
  bool mDualHeads;
  /// Determines if the arrow head(s) are shaded.
  bool mFilledHeads;
  /// Length of the arrow shaft.
  float mLength;
  /// Radius around the arrow shaft within which the Gizmo receives mouse
  /// events.
  float mSelectRadius;
  /// Arrow head size.
  float mHeadSize;
  /// Thickness of the arrow shaft.
  float mLineDrawWidth;
  /// Head type may be a box or a cone.
  ArrowHeadType::Enum mHeadType;
};

/// Notification about various rotation parameters on the most recent RingGizmo
/// modification.
class RingGizmoEvent : public GizmoUpdateEvent
{
public:
  /// Meta Initialization.
  ZilchDeclareType(RingGizmoEvent, TypeCopyMode::ReferenceType);

  /// Constructor.
  RingGizmoEvent(GizmoUpdateEvent* e);

  Quat mWorldRotation;
  Vec3 mWorldRotationAxis;
  float mRadiansAroundAxis;
  float mDeltaRadiansAroundAxis;
};

class RingGizmo : public SimpleGizmoBase
{
public:
  /// Meta Initialization.
  ZilchDeclareType(RingGizmo, TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Test if the mouse is over this gizmo.
  void OnGizmoRayTest(GizmoRayTestEvent* e);

  /// We want to update the drag direction based on our orientation.
  void OnGizmoPreDrag(GizmoEvent* e);

  /// While the gizmo is being dragged, all the user is given is an event
  /// with the world movement of the mouse. We're going to respond to that
  /// here and send out an extra event with the calculated rotation based
  /// on the mouse's movement.
  void OnGizmoModified(GizmoUpdateEvent* e);

  /// Draw the gizmo every frame.
  void OnFrameUpdate(Event*);

  Vec2 mMouseDragStart;
  Vec2 mPreviousMouseDragSample;
  Vec3 mWorldRotationAxis;

  /// Used to define the axis that the ring is around.
  Orientation* mOrientation;

  Vec3 mGrabPoint;
  Vec3 mGrabMoveAxis;

  bool mUseParentAsViewScaleOrigin;

  bool mBackShade;
  bool mViewAligned;

  /// The radius of the ring.
  float mRadius;

  /// The radius of the selection around the ring.
  float mSelectRadius;

  /// How fast the object rotates as the mouse drags. This also
  /// depends on ScreenDpi.
  float mDragRadiansPerPixel;

  /// Arrow draw properties.
  Vec4 mGrabArrowColor;
  float mGrabArrowLength;
  float mGrabArrowHeadSize;
  bool mGrabArrowViewScaled;
  bool mGrabArrowOnTop;
};

DeclareEnum3(UpdateMode, None, TranslateSelf, TranslateRoot);

/// Updates the translation of the gizmo when it's being dragged
class TranslateGizmo : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TranslateGizmo, TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  bool GetSnapping();
  void SetSnapping(bool snapping);

  /// When the mouse drag starts, we want to store our position in order
  /// to calculate our new position when the gizmo is modified.
  void OnMouseDragStart(Event* e);

  /// As the gizmo is being dragged, we want to update the Transform based
  /// on the current update mode.
  void OnGizmoModified(GizmoUpdateEvent* e);

  Vec3 TranslateFromDrag(GizmoDrag* gizmoDrag, Vec3Param startPosition, Vec3Param movement, QuatParam rotation);

  /// Start Position getter (we want it to be read only).
  Vec3 GetStartPosition();

  /// We need the transform to move the object.
  Transform* mTransform;

  /// Whether or not to automatically translate the gizmo as it's moved.
  UpdateMode::Enum mUpdateMode;

  /// The world translation of the gizmo at the start of a drag.
  Vec3 mStartPosition;

  /// Most recent drag mode used with gizmo.
  /// Copied from 'GizmoUpdateEvent' when gizmo is modified.
  GizmoDragMode::Enum mDragMode;

  /// Snapping.
  bool mSnapping;
  float mSnapDistance;
  GizmoSnapMode::Enum mSnapMode;
};

class ScaleGizmo : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(ScaleGizmo, TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  bool GetSnapping();
  void SetSnapping(bool snapping);

  /// Record where a gizmo drag began.
  void OnMouseDragStart(ViewportMouseEvent* e);

  /// As the gizmo is being dragged, we want to update all objects.
  void OnGizmoModified(GizmoUpdateEvent* e);

  /// Generate a new scale based on drag-type [ie: viewplane, gizmo-basis-plane,
  /// gizmo-axis].
  Vec3 ScaleFromDrag(GizmoBasis::Enum basis,
                     GizmoDrag* gizmoDrag,
                     float distance,
                     Vec3Param movement,
                     Vec3Param startScale,
                     MetaTransformParam transform);

  /// Used when dragging on the view axis to determine which direction
  Vec3 mEyeDirection;
  /// Drag distance on the view plane.
  float mViewPlaneMove;

  /// Most recent drag mode used with gizmo.
  /// Copied from 'GizmoUpdateEvent' when gizmo is modified.
  GizmoDragMode::Enum mDragMode;

  /// Scaled mouse movement based on grab-point vs. gizmo origin
  /// (note: movement in local gizmo-orientation).
  Vec3 mScaledLocalMovement;

  /// Snapping.
  bool mSnapping;
  float mSnapDistance;
  GizmoSnapMode::Enum mSnapMode;

  Vec3 mDirection;
};

class RotateGizmo : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(RotateGizmo, TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  bool GetSnapping();
  void SetSnapping(bool snapping);

  /// Record where a gizmo drag began.
  void OnMouseDragStart(ViewportMouseEvent* e);

  /// As the gizmo is being dragged, we want to update all objects.
  void OnGizmoModified(RingGizmoEvent* e);

  /// Snapping.
  bool mSnapping;
  float mSnapAngle;
  float mPreviousSnap;
};

} // namespace Zero
