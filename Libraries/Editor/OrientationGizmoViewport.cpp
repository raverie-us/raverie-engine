// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

OrientationGizmoViewport::OrientationGizmoViewport(EditorViewport* editorViewport) : Widget(editorViewport)
{
  mEditorViewport = editorViewport;

  GameSession* gameSession = Z::gEditor->GetEditGameSession();
  Archetype* spaceArchetype = ArchetypeManager::Find(CoreArchetypes::DefaultSpace);
  mViewCubeSpace = gameSession->CreateEditorSpace(spaceArchetype);
  mViewCubeSpace->SetName(CoreArchetypes::ViewCube);

  mCamera = mViewCubeSpace->CreateAt(CoreArchetypes::PreviewCamera, Vec3(0, 0, 2.5));
  mCamera->ClearArchetype();

  Component* renderer = mCamera->GetComponentByName("ForwardRenderer");
  if (renderer != nullptr)
  {
    Any clearColor(Vec4::cZero);
    renderer->SetProperty("ClearColor", clearColor);
    Texture* skybox = TextureManager::FindOrNull("WhiteSkybox");
    renderer->SetProperty("Skybox", skybox);
  }

  CameraViewport* cameraViewport = HasOrAdd<CameraViewport>(mCamera);
  cameraViewport->SetGameWidgetOverride(editorViewport->mGameWidget);
  cameraViewport->SetActiveCamera(mCamera->has(Camera));
  cameraViewport->mRenderOrder = Math::IntegerPositiveMax();
  cameraViewport->SetRenderToViewport(true);

  mViewport = (Viewport*)cameraViewport->mViewport;

  Cog* viewCube = mViewCubeSpace->CreateAt(CoreArchetypes::ViewCube, Vec3::cZero);
  viewCube->ClearArchetype();
  SetOrientations(*viewCube);

  ConnectThisTo(this, Events::MouseUpdate, OnMouseUpdate);
  ConnectThisTo(this, Events::LeftMouseUp, OnMouseUp);

  mRayCaster.AddProvider(new PhysicsRaycastProvider());
}

OrientationGizmoViewport::~OrientationGizmoViewport()
{
}

void OrientationGizmoViewport::OnDestroy()
{
  mViewCubeSpace->Destroy();
}

void OrientationGizmoViewport::UpdateTransform()
{
  if (mCamera.IsNull())
  {
    Widget::UpdateTransform();
    return;
  }

  CameraViewport* cameraViewport = mCamera->has(CameraViewport);

  Vec3 pos = GetTranslation();
  Vec2 size = GetSize();
  Vec2 parentSize = GetParent()->GetSize();

  Vec2 normalizeSize = Vec2(size.x / parentSize.x, size.y / parentSize.y);
  Vec2 normalizedOffset = Vec2(pos.x / parentSize.x, pos.y / parentSize.y);

  cameraViewport->mNormalizedSize = normalizeSize;
  cameraViewport->mNormalizedOffset = normalizedOffset;

  Widget::UpdateTransform();
}

void OrientationGizmoViewport::SetGizmoRotation(Quat rotation)
{
  rotation.Normalize();
  Cog* cube = mViewCubeSpace->FindObjectByName(SpecialCogNames::ViewCube);
  if (cube)
  {
    if (cube->has(Transform)->GetRotation() == rotation)
      return;
    cube->has(Transform)->SetWorldRotation(rotation);
    SetFaces(Math::Multiply(rotation.Inverted(), Vec3(0, 0, -1)));
  }
}

void OrientationGizmoViewport::OnMouseUp(MouseEvent* event)
{
  Vec3 dir;
  if (GetViewDirection(event, dir))
  {
    Cog* editorCamera = mEditorViewport->mEditorCamera;
    EditorCameraController* controller = editorCamera->has(EditorCameraController);
    if (!controller)
    {
      event->Terminate();
      return;
    }

    dir.Normalize();
    real newH = -Math::RadToDeg(Math::ArcTan2(-dir.x, -dir.z));
    real newV = Math::RadToDeg(-Math::ArcSin(dir.y));

    real oldH = controller->GetHorizontalAngle();
    real oldV = controller->GetVerticalAngle();

    real deltaH = newH - Math::FMod(oldH, 360.0f);
    if (deltaH > 180.0f)
      deltaH = (newH - 360.0f) - Math::FMod(oldH, 360.0f);
    else if (deltaH < -180.0f)
      deltaH = (newH + 360.0f) - Math::FMod(oldH, 360.0f);

    newH = oldH + deltaH;

    ActionGroup* actionGroup = new ActionGroup(controller->GetOwner(), ActionExecuteMode::FrameUpdate);
    actionGroup->Add(
        AnimatePropertyGetSet(EditorCameraController, HorizontalAngle, Ease::Quad::InOut, controller, 0.5f, newH));
    actionGroup->Add(
        AnimatePropertyGetSet(EditorCameraController, VerticalAngle, Ease::Quad::InOut, controller, 0.5f, newV));

    event->Terminate();
  }
}

void OrientationGizmoViewport::OnMouseUpdate(MouseEvent* event)
{
  Debug::ActiveDrawSpace drawSpace(mViewCubeSpace->GetRuntimeId());

  Cog* hitCog = GetRaycastResult(event);
  if (hitCog)
  {
    hitCog->has(Collider)->DebugDraw();

    event->Terminate();
  }
}

void OrientationGizmoViewport::SetOrientations(Cog& cog)
{
  forRange (Cog& child, cog.GetChildren())
  {
    Collider* collider = child.has(Collider);
    if (collider != nullptr)
    {
      Vec3 dir;
      collider->GetCenter(dir);
      mOrientationMap[CogId(&child)] = -dir.AttemptNormalized();
    }

    SetOrientations(child);
  }
}

bool OrientationGizmoViewport::GetViewDirection(MouseEvent* event, Vec3& direction)
{
  if (mViewport.IsNull())
    return false;

  RaycastResultList result(1);
  CastInfo castInfo(mViewCubeSpace, mViewport->mViewportInterface->GetCameraCog(), event->Position);
  Ray pickRay = mViewport->ScreenToWorldRay(event->Position);

  mRayCaster.RayCast(pickRay, castInfo, result);

  if (result.mSize != 0)
  {
    Cog* hitCog = result.mEntries[0].HitCog;
    auto range = mOrientationMap.Find(hitCog);
    if (range.Empty() == false)
    {
      direction = range.Front().second;
      return true;
    }
  }

  return false;
}

Cog* OrientationGizmoViewport::GetRaycastResult(MouseEvent* event)
{
  if (mViewport.IsNull())
    return nullptr;

  RaycastResultList result(1);
  CastInfo castInfo(mViewCubeSpace, mViewport->mViewportInterface->GetCameraCog(), event->Position);
  Ray pickRay = mViewport->ScreenToWorldRay(event->Position);

  mRayCaster.RayCast(pickRay, castInfo, result);

  if (result.mSize != 0)
    return result.mEntries[0].HitCog;
  else
    return nullptr;
}

void OrientationGizmoViewport::SetFaces(Vec3 direction)
{
  forRange (auto pair, mOrientationMap.All())
  {
    CogId cogId = pair.first;
    if (cogId.IsValid() == false)
      continue;

    Vec3 dir = pair.second;

    int count = 0;
    if (dir.x != 0)
      ++count;
    if (dir.y != 0)
      ++count;
    if (dir.z != 0)
      ++count;

    if (count != 1)
      continue;

    Transform* transform = cogId.has(Transform);

    real t = Math::Abs(dir.Dot(direction));
    t = 1.0f - (t / 0.5f);
    if (t < 0.0f)
      t = 0.0f;

    Vec3 in = Vec3::cZero;
    Vec3 out = -dir * 0.24f;

    transform->SetTranslation(in * (1.0f - t) + out * t);
  }
}

} // namespace Zero
