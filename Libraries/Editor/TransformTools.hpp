// MIT Licensed (see LICENSE.md).
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
} // namespace Events

class ManipulatorToolEvent : public ViewportMouseEvent
{
public:
  ZilchDeclareType(ManipulatorToolEvent, TypeCopyMode::ReferenceType);

  ManipulatorToolEvent(ViewportMouseEvent* event);

  OperationQueue* GetOperationQueue();
  bool GetFinished();

  /// Null until this event is dispatched as 'ManipulatorToolEnd'.
  HandleOf<OperationQueue> mOperationQueue;

  Location::Enum mGrabLocation;

  Rectangle mStartWorldRectangle;
  /// If marking the event as HandledEventScript == false, then modifications
  /// to 'EndWorldRectangle' (in script) will be applied internally by the
  /// ManipulatorTool.
  Rectangle mEndWorldRectangle;
};

// Store data about each object being transformed
struct TransformingObject
{
  Handle MetaObject;

  Vec3 StartWorldTranslation;
  Vec3 StartTranslation;
  Quat StartRotation;
  Vec3 StartScale;
  Vec2 StartSize;

  Vec3 StartColliderOffset;
  Vec3 StartColliderSize;

  Vec3 EndTranslation;
  Quat EndRotation;
  Vec3 EndScale;
  Vec2 EndSize;
};

/// <Commands>
///   <command name = "Maintain Aspect Ratio">
///     <shortcut> Ctrl + A + Drag (Corners) </shortcut>
///     <description>
///       While held, maintain aspect ratio of the object being manipulated.
///       Note: Only activates on corner grab nodes of the ManipulatorTool.
///     </description>
///   </command>
///   <command name = "CopySelection">
///     <shortcut> Ctrl + Drag (Center) </shortcut>
///     <description>
///       Duplicate all objects in the current selection.  Then, set the
///       duplicates to be the current selection and target of the
///       ManipulatorTool.  Note: Only activates on the center grab node of the
///       ManipulatorTool.
///     </description>
///   </command>
///   <command name = "TempSnapping">
///     <shortcut> Shift (Hold) </shortcut>
///     <description>
///       While held, temporarily switch the state of 'Snapping' to the opposite
///       of its current state.
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

  IncludeMode::Enum GetIncludeMode();
  void SetIncludeMode(IncludeMode::Enum mode);

  float GetSnapDistance();
  void SetSnapDistance(float distance);

  bool Active();
  bool CanManipulate();
  bool ActiveWithSelection();

  bool CheckMouseManipulation(ViewportMouseEvent* event);

  void ZeroOutManipulator(Vec3Param center = Vec3::cZero);

  bool PrioritizeUiWidget(Array<Handle>::range& range);
  bool CheckAlignment(Array<Handle>::range& range, Vec3Param toolNormal);
  void ComputeWorldToolRect(Array<Handle>::range& range,
                            Vec3Param toolPosition,
                            bool includeChildren,
                            bool childPass = false);
  void ComputeLocalToolRect(Array<Handle>::range& range,
                            Vec3Param toolNormal,
                            Vec3Param pointOnToolPlane,
                            Aabb* aabbOut,
                            bool includeChildren,
                            bool childPass = false);

  void ExtractPrimary(Array<Handle>::range& range, Handle& primaryOut);
  void CompileChildObjects(Cog* parent, Array<Handle>& children);
  void UpdateSelectedObjects();
  void UpdateRectAndBasis();

  void SetToolRect(const Aabb& aabb);

  /// Set the gizmo mode when activated / deactivated.
  void OnToolActivate(Event*);
  void OnToolDeactivate(Event*);

  void OnFrameUpdate(UpdateEvent*);

  /// Responds to objects having a 'Transform' component removed or added.
  void OnComponentsChanged(Event*);
  /// Responds to selection being created [ex: multi-select dragging].
  void OnSelectionChanged(Event*);
  void OnFinalSelectionChanged(Event*);

  // Event Response.
  void OnLeftMouseDown(ViewportMouseEvent* event);
  void OnLeftMouseUp(ViewportMouseEvent* event);
  void OnMouseMove(ViewportMouseEvent* event);
  void TestMouseMove(ViewportMouseEvent* event);

  void DuplicationObjects(Array<Handle>& metaObjects);
  void AddTransformingObject(Handle target, ManipulatorToolEvent& eventToSend);

  /// Gizmo Interface.
  void OnMouseDragStart(ViewportMouseEvent* event);
  void OnMouseDragMove(ViewportMouseEvent* event);
  void OnMouseDragEnd(ViewportMouseEvent* event);

  void OnToolDraw(Event*);

  /// Snap value being transformed.
  bool mSnapping;
  float mSnapDistance;

  /// Update box collier size with area.
  bool mSizeBoxCollider;

  /// Toggle duplication of objects in the ManipulatorTool's object selection
  /// when Crtl dragging the center grab node of the ManipulatorTool.
  bool mDuplicateOnCtrlDrag;

  unsigned mDuplicateCorrectIndex;

  /// Determine if manipulation will affect child objects of each object in the
  /// selection, or if manipulation affects only the selection objects.
  IncludeMode::Enum mIncludeMode;

  GizmoMode::Enum mGizmoMode;
  GizmoGrab::Enum mGrabMode;

  Vec2 mMouseDragStart;
  Vec3 mWorldGrabPoint;

  int mSelectedPoint;

  bool mValidLocation;
  Location::Enum mLocation;

  Rectangle mActiveRect;
  Rectangle mStartRect;

  Vec3 mStartCenter3D;
  Vec3 mActiveOrigin3D;
  Quat mObbBasis;

  Vec4 mToolColor;
  Vec4 mHoverColor;

  Array<Handle> mObjects;
  HashSet<Handle> mNewObjects;
  Array<TransformingObject> mTransformingObjects;
};

} // namespace Zero
