// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

namespace Events
{
DeclareEvent(ToolCreateGizmoEvent);
DeclareEvent(DuplicateFirstChance);
} // namespace Events

class ToolGizmoEvent : public Event
{
public:
  ZilchDeclareType(ToolGizmoEvent, TypeCopyMode::ReferenceType);
  ToolGizmoEvent(Cog* gizmo)
  {
    mGizmo = gizmo;
  }

public:
  Cog* mGizmo;
};

class GizmoCreator : public Component
{
public:
  ZilchDeclareType(GizmoCreator, TypeCopyMode::ReferenceType);

  GizmoCreator();

  void Serialize(Serializer& stream);
  void Initialize(CogInitializer& initializer);

  void OnDestroy(uint flags = 0) override;
  void DestroyGizmo();

  Archetype* GetGizmoArchetype();
  void SetGizmoArchetype(Archetype* gizmoArchetype);

  Cog* GetGizmo(Space* space);

public:
  HandleOf<Archetype> mGizmoArchetype;
  HandleOf<Cog> mGizmo;
  HandleOf<Space> mSpace;
};

class ObjectTransformTool : public Component
{
public:
  ZilchDeclareType(ObjectTransformTool, TypeCopyMode::ReferenceType);

  ObjectTransformTool();

  void Serialize(Serializer& stream);
  void Initialize(CogInitializer& initializer);
  void DestroyGizmo();

  void RegisterNewGizmo();

  void OnToolActivate(Event* event);
  void OnToolDeactivate(Event* event);
  void OnToolGizmoCreated(ToolGizmoEvent* event);

  virtual void OnFrameUpdate(UpdateEvent* event);

  void OnComponentsChanged(Event* event);
  void OnSelectionChanged(Event* event);
  void OnFinalSelectionChanged(Event* event);
  void OnKeyDown(KeyboardEvent* e);

  void UpdateGrabState(GizmoGrabMode::Enum state);

  GizmoGrabMode::Enum GetGrab();
  void SetGrab(GizmoGrabMode::Enum grab);

  GizmoBasis::Enum GetBasis();
  void SetBasis(GizmoBasis::Enum basis);

  GizmoPivot::Enum GetPivot();
  void SetPivot(GizmoPivot::Enum pivot);

  Space* GetSpaceFromSelection(MetaSelection* selection);

  virtual void GizmoCreated(Cog* gizmo){};
  virtual void CopyPropertiesToGizmo(){};

public:
  bool mChangeFromUs;
  CogId mGizmo;

  MetaSelection mRegisteredObjects;

  GizmoGrabMode::Enum mGrab;
  GizmoBasis::Enum mBasis;
  GizmoPivot::Enum mPivot;

  bool mSnapping;
  float mSnapDistance;
};

/// <Commands>
///   <command name = "CopySelection">
///     <shortcut> Ctrl + Drag </shortcut>
///     <description>
///       Duplicate all objects in the current selection.  Then, set the
///       duplicates to be the current selection and target of the
///       TranslateTool.
///     </description>
///   </command>
///   <command name = "SnapToSurface">
///     <shortcut> V + Drag\(Center Square) </shortcut>
///     <description>
///       Snap the center of the translate tool to the surface being hovered
///       over by the mouse cursor. Thereby affecting all target-objects of the
///       TranslateTool.
///     </description>
///   </command>
///   <command name = "TempSnapping">
///     <shortcut> Shift (Hold) </shortcut>
///     <description>
///       While held, temporarily switch the state of 'Snapping' to the opposite
///       of its current state.
///     </description>
///   </command>
///   <command name = "ToggleBasis">
///     <shortcut> X </shortcut>
///     <description>
///       Toggle the tool's basis setting between Local or World.
///     </description>
///   </command>
/// </Commands>
class ObjectTranslateTool : public ObjectTransformTool
{
public:
  ZilchDeclareType(ObjectTranslateTool, TypeCopyMode::ReferenceType);

  ObjectTranslateTool();

  void Serialize(Serializer& stream);
  void Initialize(CogInitializer& initializer);

  GizmoDragMode::Enum GetDragMode();

  bool GetSnapping();
  void SetSnapping(bool snapping);

  float GetSnapDistance();
  void SetSnapDistance(float distance);

  GizmoSnapMode::Enum GetSnapMode();
  void SetSnapMode(GizmoSnapMode::Enum mode);

  void OnGizmoObjectsDuplicated(Event* event);

  void GizmoCreated(Cog* gizmo) override;
  void CopyPropertiesToGizmo() override;

public:
  GizmoSnapMode::Enum mSnapMode;
};

/// <Commands>
///   <command name = "OffAxesScale">
///     <shortcut> Ctrl + Drag </shortcut>
///     <description>
///       When dragging one of the three main axes - scale by the drag amount
///       on the other two, non-drag axes, and not the drag axis itself.
///     </description>
///   </command>
///   <command name = "TempSnapping">
///     <shortcut> Shift (Hold) </shortcut>
///     <description>
///       While held, temporarily switch the state of 'Snapping' to the opposite
///       of its current state.
///     </description>
///   </command>
///   <command name = "ToggleBasis">
///     <shortcut> X </shortcut>
///     <description>
///       Toggle the tool's basis setting between Local or World.
///     </description>
///   </command>
/// </Commands>
class ObjectScaleTool : public ObjectTransformTool
{
public:
  ZilchDeclareType(ObjectScaleTool, TypeCopyMode::ReferenceType);

  ObjectScaleTool();

  void Serialize(Serializer& stream);
  void Initialize(CogInitializer& initializer);

  GizmoDragMode::Enum GetDragMode();

  bool GetSnapping();
  void SetSnapping(bool snapping);

  void SetSnapDistance(float distance);
  float GetSnapDistance();

  GizmoSnapMode::Enum GetSnapMode();
  void SetSnapMode(GizmoSnapMode::Enum mode);

  bool GetAffectTranslation();
  void SetAffectTranslation(bool affectTranslation);

  bool GetAffectScale();
  void SetAffectScale(bool affectScale);

  void CopyPropertiesToGizmo() override;

public:
  GizmoSnapMode::Enum mSnapMode;
  /// With multiple objects selected, allow their spacial-offest to be affected
  /// about the chosen pivot point, while being locally scaled with
  /// 'mAffectScale'.
  bool mAffectTranslation;
  /// With multiple objects selected, allow their local scale to be affected
  /// while being spacially-offset (with 'AffectTranslation') about the chosen
  /// pivot point.
  bool mAffectScale;
};

/// <Commands>
///   <command name = "TempSnapping">
///     <shortcut> Shift (Hold) </shortcut>
///     <description>
///       While held, temporarily switch the state of 'Snapping' to the opposite
///       of its current state.
///     </description>
///   </command>
///   <command name = "ToggleBasis">
///     <shortcut> X </shortcut>
///     <description>
///       Toggle the tool's basis setting between Local or World.
///     </description>
///   </command>
/// </Commands>
class ObjectRotateTool : public ObjectTransformTool
{
public:
  ZilchDeclareType(ObjectRotateTool, TypeCopyMode::ReferenceType);

  ObjectRotateTool();

  void Serialize(Serializer& stream);
  void Initialize(CogInitializer& initializer);

  bool GetSnapping();
  void SetSnapping(bool snapping);

  float GetSnapAngle();
  void SetSnapAngle(float angle);

  bool GetAffectTranslation();
  void SetAffectTranslation(bool affectTranslation);

  bool GetAffectRotation();
  void SetAffectRotation(bool affectRotation);

  void CopyPropertiesToGizmo() override;

public:
  /// With multiple objects selected, allow their spacial-offest to be rotated
  /// about the chosen pivot point, while being locally rotated with
  /// 'mAffectRotation'.
  bool mAffectTranslation;
  /// With multiple objects selected, allow their local rotation to be affected
  /// while being spacially-rotated (with 'AffectTranslation') about the chosen
  /// pivot point.
  bool mAffectRotation;
};

} // end namespace Zero
