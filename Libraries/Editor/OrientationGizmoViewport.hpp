// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class EditorViewport;

class OrientationGizmoViewport : public Widget
{
public:
  typedef OrientationGizmoViewport ZilchSelf;

  OrientationGizmoViewport(EditorViewport* editorViewport);
  ~OrientationGizmoViewport();

  void OnDestroy() override;

  void UpdateTransform() override;

  void SetGizmoRotation(Quat rotation);

  void OnMouseUp(MouseEvent* event);
  void OnMouseUpdate(MouseEvent* event);

private:
  void SetOrientations(Cog& cog);
  bool GetViewDirection(MouseEvent* event, Vec3& direction);
  void SetFaces(Vec3 direction);
  Cog* GetRaycastResult(MouseEvent* event);

  EditorViewport* mEditorViewport;
  Space* mViewCubeSpace;
  HandleOf<Cog> mCamera;
  HandleOf<Viewport> mViewport;

  HashMap<CogId, Vec3> mOrientationMap;
  Raycaster mRayCaster;
};

} // namespace Zero
