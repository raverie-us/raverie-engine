// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class Editor;
class EditorCameraController;
class ObjectPollEvent;
class OrientationGizmoViewport;
class WindowTabEvent;
class MetaDropEvent;
class SavingEvent;
class CommandCaptureContextEvent;
class ToolControl;
class EditorViewportMenu;

class RenderTasksEvent;

// Used for viewports to share spaces
DeclareEnum2(OwnerShip, Owner, Sharing);

// Mouse operations for selection and dragging
DeclareEnum4(MouseMode, Default, Select, Camera, Drag);

/// Editor Viewport. Uses tools to edit target space.
class EditorViewport : public Composite
{
public:
  ZilchDeclareType(EditorViewport, TypeCopyMode::ReferenceType);

  EditorViewport(Composite* parent, OwnerShip::Enum ownership);

  void OnDestroy() override;

  /// Clean up objects
  void CleanUp();

  /// Set this as the active viewport for playing
  void SetAsActive();

  /// Find the editor camera
  void SetUpEditorCamera();
  void OnCameraDestroyed(ObjectEvent* event);

  ReactiveViewport* GetReactiveViewport();

  // Target a space for editing
  void SetTargetSpace(Space* newSpace);
  Space* GetTargetSpace();

  // Widget Interface
  bool TakeFocusOverride() override;
  void UpdateTransform() override;
  Vec2 GetMinSize() override;

  // Shared Tools
  ToolControl* mTools;

  // Editor Camera for this viewport
  HandleOf<Cog> mEditorCamera;

  // Space being edited by this viewport
  HandleOf<Space> mEditSpace;
  OwnerShip::Enum mOwnerShip;

  // If this viewport is editing an archetype
  HandleOf<Archetype> mEditArchetype;

  MouseMode::Enum mMouseOperation;
  Vec2 mMouseDownScreenPosition;

  /// Widget that all viewports are attached to.
  GameWidget* mGameWidget;

  /// Menu widgets.
  EditorViewportMenu* mMenu;
  Element* mMenuToggleButton;
  bool mMenuHidden;

  /// Used to stop the tab from ever showing modified for this viewport
  bool mIgnoreSpaceModifications;

  OrientationGizmoViewport* mOrientationGizmo;

  void ConfigureViewCube(bool active, real viewportSize);

  // Event Handlers
  void OnMouseEnterMenuToggle(Event* e);
  void OnMouseExitMenuToggle(Event* e);
  void OnMenuToggleClicked(Event* e);
  void OnSpaceModified(Event* e);
  void OnSpaceLevelLoaded(Event* e);
  void OnSpaceDestroyed(Event* e);
  void OnTabFind(WindowTabEvent* e);
  void OnSave(SavingEvent* e);
  void OnSaveCheck(SavingEvent* e);
  void OnObjectPoll(ObjectPollEvent* e);
  void OnCaptureContext(CommandCaptureContextEvent* e);
  void OnResourcesRemoved(ResourceEvent* e);
  void OnResourceModified(ResourceEvent* event);
  void OnMouseEnter(MouseEvent* e);
  void OnMiddleMouseDown(MouseEvent* e);
  void OnRightMouseDown(MouseEvent* e);
  void OnMouseDown(MouseEvent* e);
  void OnLeftMouseDown(MouseEvent* e);
  void OnMouseDoubleClick(MouseEvent* e);
  void OnLeftMouseDrag(MouseEvent* e);
  void OnMouseUpdate(MouseEvent* e);
  void OnMouseMove(MouseEvent* e);
  void OnMouseUp(MouseEvent* e);
  void OnRightMouseUp(MouseEvent* e);
  void OnMouseScroll(MouseEvent* e);
  void OnMetaDrop(MetaDropEvent* e);
  void OnKeyDown(KeyboardEvent* e);
  void OnCutCopyPaste(ClipboardEvent* event);
  void OnKeyUp(KeyboardEvent* e);
  void OnFocusLost(FocusEvent* focusEvent);
  void OnFocusGained(FocusEvent* focusEvent);

  /// If a tool Cog isn't given, it will use the active tool.
  /// Returns whether or not the tool handled the event.
  bool ForwardMouseEventToTool(MouseEvent* e, Cog* tool = nullptr);
  bool ForwardEventToTool(MouseEvent* e, Cog* tool = nullptr);
  bool ForwardEventToTool(KeyboardEvent* e, Cog* tool = nullptr);
  bool ForwardEventToTool(ClipboardEvent* e, Cog* tool = nullptr);
  bool ForwardEventToGizmos(MouseEvent* e);
  bool ForwardEventToGizmos(KeyboardEvent* e);
  bool ForwardEventToGizmos(ClipboardEvent* e);

  void OnCameraUpdate(ObjectEvent* event);
  void OnSettingsChanged(Event* event);
};

} // namespace Zero
