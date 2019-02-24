// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class EditorCameraMouseDrag : public MouseManipulation
{
public:
  typedef EditorCameraMouseDrag ZilchSelf;
  HandleOf<EditorCameraController> mController;
  HandleOf<Viewport> mViewport;
  HandleOf<EditorViewport> mEditorViewport;

  EditorCameraMouseDrag(Mouse* mouse, EditorViewport* editorViewport, EditorCameraController* controller);
  EditorCameraMouseDrag(Mouse* mouse, Viewport* viewport, EditorCameraController* controller);
  virtual ~EditorCameraMouseDrag();

  // Generic MouseEvent Handlers
  void OnMouseMove(MouseEvent* event) override;
  void OnMouseUp(MouseEvent* event) override;
  void OnMouseDown(MouseEvent* event) override;

  // Named MouseUp Event Handlers
  void OnRightMouseUp(MouseEvent* event) override;
  void OnMiddleMouseUp(MouseEvent* event) override;

  // Name MouseDown Event Handlers
  void OnRightMouseDown(MouseEvent* event) override;
  void OnMiddleMouseDown(MouseEvent* event) override;

  void OnKeyDown(KeyboardEvent* event) override;
  void OnKeyUp(KeyboardEvent* event) override;
  void OnTargetDestroy(MouseEvent* event) override;
};

} // namespace Zero
