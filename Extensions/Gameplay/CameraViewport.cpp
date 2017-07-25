///////////////////////////////////////////////////////////////////////////////
///
/// \file CameraViewport.cpp
///
/// Authors: Nathan Carlson, Chris Peters
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(CameraViewport, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(RenderInEditor);
  ZilchBindGetterSetterProperty(RenderInGame);
  ZilchBindFieldProperty(mRenderOrder);
  ZilchBindFieldProperty(mCameraPath)->ZeroBindPropertyRename("Camera");
  ZilchBindFieldProperty(mRendererPath)->ZeroBindPropertyRename("Renderer");
  ZilchBindGetterSetterProperty(ResolutionOrAspect);
  ZilchBindGetterSetterProperty(RenderToViewport)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindFieldProperty(mForwardViewportEvents)->ZeroFilterEquality(mRenderToViewport, bool, true);;
  ZilchBindFieldProperty(mNormalizedSize)->ZeroFilterEquality(mRenderToViewport, bool, true);
  ZilchBindFieldProperty(mNormalizedOffset)->ZeroFilterEquality(mRenderToViewport, bool, true);
  ZilchBindFieldProperty(mViewportScaling)->ZeroFilterEquality(mRenderToViewport, bool, true);
  ZilchBindFieldProperty(mMarginColor)->ZeroFilterEquality(mRenderToViewport, bool, true);

  ZilchBindMethod(ViewportTakeFocus);
  ZilchBindGetter(ViewportHasFocus);

  ZilchBindGetter(WorldToView);
  ZilchBindGetter(ViewToPerspective);
  ZilchBindGetter(WorldToPerspective);
  ZilchBindGetter(FinalTexture);

  ZilchBindGetter(MouseWorldRay);
  ZilchBindMethod(ScreenToWorldRay);
  ZilchBindMethod(ScreenToWorldZPlane);
  ZilchBindMethod(ScreenToWorldViewPlane);
  ZilchBindMethod(ScreenToWorldPlane);
  ZilchBindMethod(WorldToScreen);
  ZilchBindMethod(ScreenToViewport);
  ZilchBindMethod(ViewportToScreen);
  ZilchBindMethod(ViewPlaneSize);

  ZilchBindGetter(ViewportResolution);
  ZilchBindGetter(ViewportResolutionWithMargin);
  ZilchBindGetter(ViewportOffset);
  ZilchBindGetter(ViewportOffsetWithMargin);
}

void CameraViewport::Serialize(Serializer& stream)
{
  SerializeNameDefault(mRenderInEditor, false);
  SerializeNameDefault(mRenderInGame, true);
  SerializeNameDefault(mRenderToViewport, true);
  SerializeNameDefault(mForwardViewportEvents, false);
  SerializeNameDefault(mRenderOrder, 0);
  SerializeNameDefault(mNormalizedSize, Vec2(1, 1));
  SerializeNameDefault(mNormalizedOffset, Vec2(0, 0));
  stream.SerializeFieldDefault("CameraPath", mCameraPath, CogPath("."), "Camera");
  stream.SerializeFieldDefault("RendererPath", mRendererPath, CogPath("."), "Renderer");
  SerializeNameDefault(mResolution, Vec2(1920, 1080));
  SerializeEnumNameDefault(ViewportScaling, mViewportScaling, ViewportScaling::Fill);
  SerializeNameDefault(mMarginColor, Vec4(0, 0, 0, 1));
}

void CameraViewport::Initialize(CogInitializer& initializer)
{
  mActiveCamera = nullptr;
  mViewport = nullptr;
  mCompleteSetup = false;

  ConnectThisTo(GetSpace(), Events::UpdateActiveCameras, OnUpdateActiveCameras);
  ConnectThisTo(&mCameraPath, Events::CogPathCogChanged, OnCameraPathChanged);

  Space* space = initializer.mSpace;
  mGraphicsSpace = space->has(GraphicsSpace);

  mFinalTexture = Texture::CreateRuntime();
  Texture* texture = mFinalTexture;
  texture->Upload(0, 0, TextureFormat::None, nullptr, 0, false);
  texture->mProtected = true;
}

void CameraViewport::OnAllObjectsCreated(CogInitializer& initializer)
{
  mCameraPath.RestoreLink(initializer, this, "CameraPath");
  mRendererPath.RestoreLink(initializer, this, "RendererPath");
}

void CameraViewport::OnDestroy(uint flags)
{
  SetActiveCamera(nullptr);
}

void CameraViewport::DebugDraw()
{
  if (mActiveCamera == nullptr)
    return;

  if (GetSpace()->IsEditorMode() && mRenderInEditor && mRenderToViewport)
    return;

  Frustum frustum = mActiveCamera->GetFrustum(GetAspectRatio());
  gDebugDraw->Add(Debug::Frustum(frustum).Color(Color::Cyan).BackShade(true));
}

float CameraViewport::GetAspectRatio()
{
  Vec2 size = GetViewportSize();
  return Math::Max(size.x, 1.0f) / Math::Max(size.y, 1.0f);
}

Vec2 CameraViewport::GetViewportSize()
{
  Viewport* viewport = mViewport;
  if (viewport != nullptr)
    return viewport->GetSize();
  else
    return GetResolutionOrAspect();
}

void CameraViewport::SendSortEvent(GraphicalSortEvent* event)
{
  GetOwner()->DispatchEvent(Events::GraphicalSort, event);
}

Cog* CameraViewport::GetCameraCog()
{
  if (mActiveCamera != nullptr)
    return mActiveCamera->GetOwner();
  return nullptr;
}

bool CameraViewport::GetRenderInEditor()
{
  return mRenderInEditor;
}

void CameraViewport::SetRenderInEditor(bool render)
{
  mRenderInEditor = render;
  CheckSetup();
}

bool CameraViewport::GetRenderInGame()
{
  return mRenderInGame;
}

void CameraViewport::SetRenderInGame(bool render)
{
  mRenderInGame = render;
  CheckSetup();
}

bool CameraViewport::GetRenderToViewport()
{
  return mRenderToViewport;
}

void CameraViewport::SetRenderToViewport(bool render)
{
  mRenderToViewport = render;
  CheckSetup();
}

Vec2 CameraViewport::GetResolutionOrAspect()
{
  return mResolution;
}

void CameraViewport::SetResolutionOrAspect(Vec2 resolution)
{
  mResolution = Math::Max(resolution, Vec2(1.0f));
}

Mat4 CameraViewport::GetWorldToView()
{
  if (mActiveCamera != nullptr)
  {
    mActiveCamera->SetAspectRatio(GetAspectRatio());
    Mat4 worldToView = mActiveCamera->GetViewTransform();
    return worldToView;
  }
  else
  {
    return Mat4::cIdentity;
  }
}

Mat4 CameraViewport::GetViewToPerspective()
{
  if (mActiveCamera != nullptr)
  {
    mActiveCamera->SetAspectRatio(GetAspectRatio());
    Mat4 viewToPerspective = mActiveCamera->GetPerspectiveTransform();
    return viewToPerspective;
  }
  else
  {
    return Mat4::cIdentity;
  }
}

Mat4 CameraViewport::GetWorldToPerspective()
{
  if (mActiveCamera != nullptr)
  {
    mActiveCamera->SetAspectRatio(GetAspectRatio());
    Mat4 worldToView = mActiveCamera->GetViewTransform();
    Mat4 viewToPerspective = mActiveCamera->GetPerspectiveTransform();
    Mat4 worldToPerspective = viewToPerspective * worldToView;
    return worldToPerspective;
  }
  else
  {
    return Mat4::cIdentity;
  }
}

HandleOf<Texture> CameraViewport::GetFinalTexture()
{
  return mFinalTexture;
}

bool CameraViewport::GetViewportHasFocus()
{
  Viewport* viewport = mViewport;
  if (viewport)
    return viewport->HasFocus();
  return false;
}

bool CameraViewport::ViewportTakeFocus()
{
  Viewport* viewport = mViewport;
  if (viewport != nullptr)
  {
    viewport->HardTakeFocus();
    return true;
  }
  return false;
}

Ray CameraViewport::GetMouseWorldRay()
{
  Mouse* mouse = Mouse::GetInstance();
  Viewport* viewport = mViewport;
  if (viewport != nullptr)
    return viewport->ScreenToWorldRay(mouse->GetClientPosition());
  else
    return Ray(Vec3::cZero, Vec3::cZAxis);
}

Ray CameraViewport::ScreenToWorldRay(Vec2Param screenPoint)
{
  Viewport* viewport = mViewport;
  if (viewport != nullptr)
    return viewport->ScreenToWorldRay(screenPoint);
  else
    return Ray(Vec3::cZero, Vec3::cZAxis);
}

Vec3 CameraViewport::ScreenToWorldZPlane(Vec2Param screenPoint, float worldDepth)
{
  Viewport* viewport = mViewport;
  if (viewport != nullptr)
    return viewport->ScreenToWorldZPlane(screenPoint, worldDepth);
  else
    return Vec3::cZero;
}

Vec3 CameraViewport::ScreenToWorldViewPlane(Vec2Param screenPoint, float viewDepth)
{
  Viewport* viewport = mViewport;
  if (viewport != nullptr)
    return viewport->ScreenToWorldViewPlane(screenPoint, viewDepth);
  else
    return Vec3::cZero;
}

Vec3 CameraViewport::ScreenToWorldPlane(Vec2Param screenPoint, Vec3Param worldPlaneNormal, Vec3Param worldPlanePosition)
{
  Viewport* viewport = mViewport;
  if (viewport != nullptr)
    return viewport->ScreenToWorldPlane(screenPoint, worldPlaneNormal, worldPlanePosition);
  else
    return Vec3::cZero;
}

Vec2 CameraViewport::WorldToScreen(Vec3Param worldPoint)
{
  Viewport* viewport = mViewport;
  if (viewport != nullptr)
    return viewport->WorldToScreen(worldPoint);
  else
    return Vec2::cZero;
}

Vec2 CameraViewport::ScreenToViewport(Vec2Param screenPoint)
{
  Viewport* viewport = mViewport;
  if (viewport != nullptr)
    return viewport->ScreenToViewport(screenPoint);
  else
    return Vec2::cZero;
}

Vec2 CameraViewport::ViewportToScreen(Vec2Param viewportPoint)
{
  Viewport* viewport = mViewport;
  if (viewport != nullptr)
    return viewport->ViewportToScreen(viewportPoint);
  else
    return Vec2::cZero;
}

Vec2 CameraViewport::ViewPlaneSize(float viewDepth)
{
  Viewport* viewport = mViewport;
  if (viewport != nullptr)
    return viewport->ViewPlaneSize(viewDepth);
  else
    return Vec2::cZero;
}

Vec2 CameraViewport::GetViewportResolution()
{
  Viewport* viewport = mViewport;
  if (viewport != nullptr)
    return viewport->GetSize();
  else
    return Vec2::cZero;
}

Vec2 CameraViewport::GetViewportResolutionWithMargin()
{
  Viewport* viewport = mViewport;
  if (viewport != nullptr)
  {
    Vec2 size = viewport->GetSize();
    size.y += viewport->mMargin[0]->GetSize().y;
    size.y += viewport->mMargin[1]->GetSize().y;
    size.x += viewport->mMargin[2]->GetSize().x;
    size.x += viewport->mMargin[3]->GetSize().x;
    return size;
  }
  else
    return Vec2::cZero;
}

Vec2 CameraViewport::GetViewportOffset()
{
  Viewport* viewport = mViewport;
  if (viewport != nullptr)
    return Math::ToVector2(viewport->GetTranslation());
  else
    return Vec2::cZero;
}

Vec2 CameraViewport::GetViewportOffsetWithMargin()
{
  Viewport* viewport = mViewport;
  if (viewport != nullptr)
    return Math::ToVector2(viewport->mMargin[0]->GetTranslation());
  else
    return Vec2::cZero;
}

void CameraViewport::OnRenderTasksUpdateInternal(RenderTasksEvent* event)
{
  ConfigureViewport();

  if (mGraphicsSpace->mActive == false || mActiveCamera == nullptr)
    return;

  // Do not process rendering if game widget is not active, does not prevent visibility events.
  // Will not have expected output if another game session is using this CameraViewport's output.
  //GameWidget* gameWidget = GetGameWidget();
  //if (gameWidget != nullptr && gameWidget->GetGlobalActive() == false)
  //  return;

  // The above logic doesn't handle preview spaces correctly when the EditorViewport is not active.
  // Temporarily just disable rendering of outputs going to the viewport, other rendering in a space will not be disabled.
  if (mViewport != nullptr && mViewport->GetGlobalActive() == false)
    return;

  EventDispatcher* rendererDispatcher = nullptr;
  if (Cog* renderer = mRendererPath.GetCog())
    rendererDispatcher = renderer->GetDispatcher();
  else
    return;

  event->mCameraViewportCog = GetOwner();
  event->mViewportSize = Math::ToIntVec2(GetViewportSize());
  event->mCamera = mActiveCamera;

  RenderTasksUpdateData update =
  {
    event,
    rendererDispatcher,
    mGraphicsSpace,
    mActiveCamera,
    mRenderOrder,
    0
  };

  // Set FinalTexture for this CameraViewport before sending event to script
  event->mFinalTexture = mFinalTexture;
  RenderTasksUpdateHelper(update);

  // Delete RenderTargets after every event is sent out
  // It is invalid to use a RenderTarget outside the scope of the RenderTasks event
  Z::gEngine->has(GraphicsEngine)->ClearRenderTargets();

  Viewport* viewport = mViewport;
  if (viewport != nullptr && update.mTaskCount != 0)
    viewport->mViewportTexture = mFinalTexture;
}

void CameraViewport::OnUpdateActiveCameras(Event* event)
{
  Camera* camera = mCameraPath.has(Camera);
  SetActiveCamera(camera);
}

void CameraViewport::OnCameraPathChanged(CogPathEvent* event)
{
  Camera* camera = mCameraPath.has(Camera);
  SetActiveCamera(camera);
}

void CameraViewport::OnCameraDestroyed(ObjectEvent* event)
{
  SetActiveCamera(nullptr);
}

void CameraViewport::CreateViewport(Camera* camera)
{
  Space* space = GetSpace();

  // Prevent camera objects in preview spaces from creating viewports
  if (space->IsPreviewMode())
    return;

  GameWidget* gameWidget = GetGameWidget();

  // Trying to create a viewport but GameWidget has been destroyed
  if (gameWidget == nullptr)
    return;

  Viewport* viewport = new ReactiveViewport(gameWidget, space, camera, this);
  viewport->mViewportInterface = this;
  mViewport = viewport;
}

void CameraViewport::ConfigureViewport()
{
  Viewport* viewport = mViewport;
  if (viewport == nullptr)
    return;

  viewport->mViewportTexture = nullptr;

  Composite* parent = viewport->GetParent();
  Vec2 parentSize = parent->GetSize();

  Vec2 viewportSize = SnapToPixels(parentSize * mNormalizedSize);
  Vec3 viewportOffset = SnapToPixels(Vec3(parentSize * mNormalizedOffset, 0.0f));

  Vec2 scaledSize = Vec2(1, 1);
  Vec3 scaledOffset = Vec3(0, 0, 0);

  if (mViewportScaling == ViewportScaling::Fill)
  {
    scaledSize = viewportSize;
    scaledOffset = viewportOffset;
  }
  else if (mViewportScaling == ViewportScaling::Letterbox)
  {
    float viewportRatio = viewportSize.x / viewportSize.y;
        
    Vec2 targetSize = SnapToPixels(mResolution);
    float targetRatio = targetSize.x / targetSize.y;
        
    Vec2 scaleAspect;
    if (targetRatio < viewportRatio)
      scaleAspect = Vec2(targetRatio / viewportRatio, 1);
    else
      scaleAspect = Vec2(1, viewportRatio / targetRatio);
    
    scaledSize = SnapToPixels(viewportSize * scaleAspect);
    scaledOffset = SnapToPixels(viewportOffset + Vec3((viewportSize - scaledSize) * 0.5f, 0.0f));
  }
  else if (mViewportScaling == ViewportScaling::Exact)
  {
    scaledSize = Math::Min(SnapToPixels(mResolution), viewportSize);
    scaledOffset = viewportOffset + SnapToPixels(Vec3((viewportSize - scaledSize) * 0.5f, 0.0f));
  }
  else if (mViewportScaling == ViewportScaling::LargestMultiple)
  {
    Vec2 targetSize = SnapToPixels(mResolution);
    float multiple = Math::Min(viewportSize.x / targetSize.x, viewportSize.y / targetSize.y);
    multiple = Math::Max(Math::Floor(multiple), 1.0f);
    scaledSize = Math::Min(targetSize * multiple, viewportSize);
    scaledOffset = viewportOffset + SnapToPixels(Vec3((viewportSize - scaledSize) * 0.5f, 0.0f));
  }

  viewport->SetSize(viewportSize, scaledSize);
  viewport->SetTranslation(viewportOffset, scaledOffset);
  viewport->SetMarginColor(mMarginColor);
}

void CameraViewport::CheckSetup()
{
  bool inEditor = GetSpace()->IsEditorMode();

  // If it shouldn't render
  if (inEditor && !mRenderInEditor || !inEditor && !mRenderInGame)
  {
    // Undo setup if it was previously completed
    ClearSetup();
  }
  else if (mCompleteSetup)
  {
    // If it should still render but camera was removed
    Viewport* viewport = mViewport;
    if (mActiveCamera == nullptr)
    {
      mViewport.SafeDestroy();
      GetSpace()->GetDispatcher()->DisconnectEvent(Events::RenderTasksUpdateInternal, this);
      mCompleteSetup = false;
    }
    // Or viewport requirement was changed
    else if (mRenderToViewport && viewport == nullptr)
      CreateViewport(mActiveCamera);
    else if (!mRenderToViewport)
      mViewport.SafeDestroy();
  }
  else if (mActiveCamera != nullptr)
  {
    if (mRenderToViewport)
      CreateViewport(mActiveCamera);
    mGraphicsSpace->AddCamera(mActiveCamera);
    ConnectThisTo(GetSpace(), Events::RenderTasksUpdateInternal, OnRenderTasksUpdateInternal);
    mCompleteSetup = true;
  }
  // No camera, can't complete setup
}

void CameraViewport::ClearSetup()
{
  if (mCompleteSetup)
  {
    mCompleteSetup = false;
    mViewport.SafeDestroy();
    // Setup could have only been completed if camera was valid and added to graphics space
    mGraphicsSpace->RemoveCamera(mActiveCamera);
    GetSpace()->GetDispatcher()->DisconnectEvent(Events::RenderTasksUpdateInternal, this);
  }
}

void CameraViewport::SetActiveCamera(Camera* camera)
{
  if (camera == mActiveCamera)
    return;

  if (mActiveCamera != nullptr)
  {
    // If rendering was active with this camera, undo setup
    if (mCompleteSetup)
    {
      Viewport* viewport = mViewport;
      if (viewport != nullptr)
        viewport->mViewportInterface = nullptr;
      mViewport.SafeDestroy();
      mGraphicsSpace->RemoveCamera(mActiveCamera);
      GetSpace()->GetDispatcher()->DisconnectEvent(Events::RenderTasksUpdateInternal, this);
      mCompleteSetup = false;
    }

    mActiveCamera->GetDispatcher()->DisconnectEvent(Events::CameraDestroyed, this);
    mActiveCamera->mViewportInterface = nullptr;
  }

  mActiveCamera = camera;

  if (mActiveCamera != nullptr)
  {
    // Handle taking a camera from another CameraViewport
    if (mActiveCamera->mViewportInterface != nullptr)
    {
      // CameraViewport is the only ViewportInterface
      CameraViewport* cameraViewport = (CameraViewport*)mActiveCamera->mViewportInterface;
      // Remove path from other CameraViewport to prevent retaking of camera
      // Changing the CogPath will also make the other CameraViewport tear-down its setup
      cameraViewport->mCameraPath.SetPath(String());
    }

    mActiveCamera->mViewportInterface = this;
    ConnectThisTo(mActiveCamera, Events::CameraDestroyed, OnCameraDestroyed);
  }

  CheckSetup();
}

void CameraViewport::SetGameWidgetOverride(GameWidget* gameWidget)
{
  mGameWidgetOverride = gameWidget;
  ClearSetup();
  CheckSetup();
}

GameWidget* CameraViewport::GetGameWidget()
{
  Space* space = GetSpace();
  GameWidget* gameWidget = mGameWidgetOverride;
  if (gameWidget == nullptr)
    gameWidget = space->mGameWidgetOverride;
  if (gameWidget == nullptr)
    gameWidget = space->GetGameSession()->mGameWidget;

  return gameWidget;
}

} // namespace Zero
