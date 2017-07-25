/// Authors: Nathan Carlson, Chris Peters
/// Copyright 2010-2016, DigiPen Institute of Technology
#include "Precompiled.hpp"

namespace Zero
{

//--------------------------------------------------------- Viewport Mouse Event

//******************************************************************************
ZilchDefineType(ViewportMouseEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindFieldProperty(mWorldRay);
  ZilchBindFieldProperty(mRayStart);
  ZilchBindFieldProperty(mRayDirection);
  ZilchBindFieldProperty(mHitPosition);
  ZilchBindFieldProperty(mHitNormal);
  ZilchBindFieldProperty(mHitDistance);
  ZilchBindGetterProperty(HitObject);
  ZilchBindGetterProperty(CameraViewport);

  ZilchBindMethod(ToWorldZPlane);
  ZilchBindMethod(ToWorldViewPlane);
  ZilchBindMethod(ToWorldPlane);
}

//******************************************************************************
ViewportMouseEvent::ViewportMouseEvent() :
  mRayStart(Vec3::cZero),
  mRayDirection(Vec3::cZero),
  mHitPosition(Vec3::cZero),
  mHitNormal(Vec3::cZero),
  mHitDistance(0.0f)
{
}

//******************************************************************************
ViewportMouseEvent::ViewportMouseEvent(MouseEvent* event) :
  mRayStart(Vec3::cZero),
  mRayDirection(Vec3::cZero),
  mHitPosition(Vec3::cZero),
  mHitNormal(Vec3::cZero),
  mHitDistance(0.0f)
{
  // Copy the event into ourselves
  *(MouseEvent*)this = *event;
}

//******************************************************************************
Cog* ViewportMouseEvent::GetHitObject()
{
  return mHitObject;
}

//******************************************************************************
CameraViewport* ViewportMouseEvent::GetCameraViewport()
{
  return mCameraViewportCog.has(CameraViewport);
}

//******************************************************************************
Vec3 ViewportMouseEvent::ToWorldZPlane(float worldDepth)
{
  Viewport* viewport = mViewport;
  if(viewport == nullptr)
    return Vec3::cZero;

  return viewport->ScreenToWorldZPlane(Position, worldDepth);
}

//******************************************************************************
Vec3 ViewportMouseEvent::ToWorldViewPlane(float viewDepth)
{
  Viewport* viewport = mViewport;
  if(viewport == nullptr)
    return Vec3::cZero;

  return viewport->ScreenToWorldViewPlane(Position, viewDepth);
}

//******************************************************************************
Vec3 ViewportMouseEvent::ToWorldPlane(Vec3Param worldPlaneNormal,
                                      Vec3Param worldPlanePosition)
{
  Viewport* viewport = mViewport;
  if(viewport == nullptr)
    return Vec3::cZero;

  return viewport->ScreenToWorldPlane(Position, worldPlaneNormal, worldPlanePosition);
}

//------------------------------------------------------------ Reactive Viewport

//******************************************************************************
ZilchDefineType(ReactiveViewport, builder, type)
{
}

//******************************************************************************
ReactiveViewport::ReactiveViewport(Composite* parent, Space* space, Camera* camera,
                                   CameraViewport* cameraViewport)
  : Viewport(parent, space, camera)
{
  mReactiveRay = Ray(Vec3::cZero, Vec3::cZAxis);
  mReactiveHitPosition = Vec3::cZero;
  mReactiveHitNormal = Vec3::cZAxis;
  mReactiveHitDistance = 0.0f;
  mCameraViewport = cameraViewport;

  mGameWidget = Type::DynamicCast<GameWidget*>(parent);
  mName = camera->GetOwner()->GetName();

  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);
  ConnectThisTo(this, Events::MouseUpdate, OnMouseGeneric);

  //Forwarded Mouse Events
  ConnectThisTo(this, Events::MouseScroll, OnMouseGeneric);
  ConnectThisTo(this, Events::MouseMove, OnMouseGeneric);

  ConnectThisTo(this, Events::DoubleClick, OnMouseGeneric);
  ConnectThisTo(this, Events::LeftClick, OnMouseGeneric);

  ConnectThisTo(this, Events::MouseDown, OnMouseGeneric);
  ConnectThisTo(this, Events::MouseUp, OnMouseGeneric);

  ConnectThisTo(this, Events::LeftMouseDown, OnMouseGeneric);
  ConnectThisTo(this, Events::LeftMouseUp, OnMouseGeneric);
  ConnectThisTo(this, Events::RightMouseDown, OnMouseGeneric);
  ConnectThisTo(this, Events::RightMouseUp, OnMouseGeneric);
  ConnectThisTo(this, Events::MiddleMouseUp, OnMouseGeneric);
  ConnectThisTo(this, Events::MiddleMouseDown, OnMouseGeneric);
}

//******************************************************************************
ReactiveSpace* ReactiveViewport::GetReactiveSpace()
{
  // Get the target space
  Space* space = mTargetSpace;//mRenderView->GetTargetSpace();

  if (space == nullptr)
    return nullptr;

  // Grab the reactive space component, which stores who we're hovering over per space
  ReactiveSpace* reactiveSpace = space->has(ReactiveSpace);
  if (reactiveSpace == nullptr)
  {
    space->AddComponentByName("ReactiveSpace");
    reactiveSpace = space->has(ReactiveSpace);
    ReturnIf(reactiveSpace == nullptr, nullptr, "Unable to add ReactiveSpace component");
  }

  return reactiveSpace;
}

//******************************************************************************
void ReactiveViewport::OnMouseEnter(MouseEvent* e)
{
  ReactiveSpace* reactiveSpace = GetReactiveSpace();
  if(!reactiveSpace)
    return;

  UpdateOverObject(e);

  ViewportMouseEvent viewportEvent(e);
  InitViewportEvent(viewportEvent);

  mCamera->GetOwner()->GetDispatcher()->Dispatch(Events::MouseEnter, &viewportEvent);
}

//******************************************************************************
void ReactiveViewport::OnMouseExit(MouseEvent* e)
{
  ReactiveSpace* reactiveSpace = GetReactiveSpace();
  if (!reactiveSpace)
    return;

  Cog* overObject = reactiveSpace->mOver;

  if(overObject)
  {
    Reactive* reactive = overObject->has(Reactive);

    if(reactive)
    {
      ViewportMouseEvent viewportEvent(e);
      InitViewportEvent(viewportEvent);

      EventDispatcher* dispatcher = overObject->GetDispatcher();

      // Always make sure that, before we dispatch mouse exit, we release the 'over' object
      // so that any script checking for the hover object will not think its this
      reactiveSpace->mOver = CogId();
      dispatcher->Dispatch(Events::MouseExit, &viewportEvent);
    }
  }
}

//******************************************************************************
void ReactiveViewport::OnMouseUpdate(MouseEvent* e)
{
  UpdateOverObject(e);

  ForwardReactiveEvent(Events::MouseUpdate, e);
}

//******************************************************************************
void ReactiveViewport::OnMouseGeneric(MouseEvent* e)
{
  ForwardReactiveEvent(e->EventId, e);
}

//******************************************************************************
bool CanClearOldReactive(Handle objectHit)
{
  if(Cog* cog = objectHit.Get<Cog*>())
  {
    Reactive* reactive = cog->has(Reactive);
    if(reactive && !reactive->mActive)
      return true;
    else if(!reactive)
      return true;

    return false;
  }

  return true;
}

//******************************************************************************
void ReactiveViewport::UpdateOverObject(MouseEvent* e)
{
  // If we have no space then don't do anything
  Space* space = mTargetSpace;
  if(!space)
    return;

  Vec2 mousePosition = e->Position;
  Ray ray = ScreenToWorldRay(mousePosition);
  this->mReactiveRay = ray;

  // Set up the filter for raycasting
  CastFilter filter;
  filter.Set(BaseCastFilterFlags::GetContactNormal);

  // Grab the reactive space component, which stores who we're hovering over per space
  ReactiveSpace* reactiveSpace = GetReactiveSpace();
  if(!reactiveSpace)
    return;

  // Keep track of the current hit object and the last hit object
  Handle objectHit;
  Handle oldOver = reactiveSpace->mOver.ToCog();

  // As long as someone else didn't handle this mouse event (blocked by another reactive in a different viewport)
  // We need to still update, but act as if we just lost any reactive object
  if(!e->Handled)
  {
    RaycastResultList results(256);
    CastInfo info(space, mViewportInterface->GetCameraCog(), mousePosition);
    reactiveSpace->mRaycaster.RayCast(ray, info, results);

    // We want to find the first object we hit, but we may hit objects without reactive
    // components, therefore we loop over a larger set of objects from raycasting and stop
    // when we hit the first one with a reactive component.
    for(size_t i = 0; i < results.mSize; ++i)
    {
      RayCastEntry& result = results.mEntries[i];
      Handle newObject(result.HitCog);
      //DebugPrint("Hit: %s\n", newObject->GetName().c_str());

      objectHit = newObject;

      //we only care about this object if it is an active reactive object.
      Reactive* reactive = newObject.IsNotNull() ? newObject.Get<Cog*>()->has(Reactive) : nullptr;
      if(reactive && reactive->mActive)
      {
        //store the mouse position info on the reactive component
        this->mReactiveHitPosition = result.HitWorldPosition;
        this->mReactiveHitNormal = result.HitWorldNormal;
        this->mReactiveHitDistance = result.T;

        // Create a reactive event that adds extra data to the mouse event
        ViewportMouseEvent viewportEvent(e);
        InitViewportEvent(viewportEvent);

        //objectHit = newObject;
        // If we hit a new object and we were hitting an old object,
        // we need to send a mouse enter to the new object and
        // a mouse exit to the old object
        if(oldOver != newObject)
        {
          EventDispatcher* newDispatcher = (newObject.Get<Object*>())->GetDispatcher( );

          if(newDispatcher)
            newDispatcher->Dispatch(Events::MouseEnterPreview, &viewportEvent);

          // Always make sure that, before we dispatch mouse exit, we release the 'over' object
          // so that any script checking for the hover object will not think its this
          reactiveSpace->mOver = newObject.Get<Cog*>();

          if(Object* oldOverObject = oldOver.Get<Object*>())
            oldOverObject->GetDispatcher()->Dispatch(Events::MouseExit, &viewportEvent);

          newDispatcher->Dispatch(Events::MouseEnter, &viewportEvent);

          return;
        }

        // Since we only care about the first object, exit now
        break;
      }
    }
  }

  // If we were hitting an old object and we don't have a new object,
  // or if the object we hit doesn't have a reactive component, or it has an inactive reactive,
  // then send a mouse exit to the old object and clear our current hover object.
  if(CanClearOldReactive(objectHit))
  {
    // Always make sure that, before we dispatch mouse exit, we release the 'over' object
    // so that any script checking for the hover object will not think its this
    reactiveSpace->mOver = CogId();
    if(Object* oldOverObject = oldOver.Get<Object*>())
    {
      ViewportMouseEvent viewportEvent(e);
      InitViewportEvent(viewportEvent);
      oldOverObject->GetDispatcher()->Dispatch(Events::MouseExit, &viewportEvent);
    }
  }
}

//******************************************************************************
void ReactiveViewport::ForwardReactiveEvent(StringParam eventName, MouseEvent* e)
{
  if (e->Handled)
    return;

  // Because widgets are destructed at different times than objects,
  // have to check for mViewportInterface to know if it's supposed to be active
  if (mViewportInterface != nullptr)
  {
    if(eventName == Events::MouseUpdate)
      UpdateOverObject(e);

    // Check Reactive
    ReactiveSpace* reactiveSpace = GetReactiveSpace();
    if (!reactiveSpace)
      return;

    // Create the event to send
    ViewportMouseEvent viewportEvent(e);
    InitViewportEvent(viewportEvent);

    mViewportInterface->GetOwner()->DispatchEvent(eventName, &viewportEvent);

    // If Camera component is not on the same object as CameraViewport
    // then send event on Camera's owner also for ease of use
    Cog* cameraCog = mViewportInterface->GetCameraCog();
    if (cameraCog != nullptr && cameraCog != mViewportInterface->GetOwner())
      cameraCog->DispatchEvent(eventName, &viewportEvent);

    // Dispatch the event on the space
    Space* space = reactiveSpace->GetSpace();
    space->DispatchEvent(eventName, &viewportEvent);

    // Dispatch on the object the mouse is over
    if(reactiveSpace->mOver)
    {
      EventDispatcher* dispatcher = reactiveSpace->mOver.ToCog()->GetDispatcher();
      dispatcher->Dispatch(eventName, &viewportEvent);
    }

    // We need to copy over whether or not the viewport event was handled to
    // the regular mouse event because it will bubble upwards, and we want
    // to properly notify that we've done something with the event
    e->Handled = viewportEvent.HandledEventScript || viewportEvent.Handled;
  }

  bool forwardToChildren = true;
  if(CameraViewport* camViewport = mCameraViewport)
    forwardToChildren = camViewport->mForwardViewportEvents;
  
  if(mGameWidget && forwardToChildren && !e->Handled)
  {
    ReactiveViewport* viewport = mGameWidget->GetViewportUnder(this);
    if(viewport)
    {
      // Make sure the mouse is actually within the viewport below us
      Rect childRect = viewport->GetScreenRect();
      if(childRect.Contains(e->Position))
        viewport->ForwardReactiveEvent(eventName, e);
    }
  }
}

//******************************************************************************
void ReactiveViewport::InitViewportEvent(ViewportMouseEvent& viewportEvent)
{
  this->mReactiveRay = ScreenToWorldRay(viewportEvent.Position);

  viewportEvent.mHitDistance = this->mReactiveHitDistance;
  viewportEvent.mHitNormal = this->mReactiveHitNormal;
  viewportEvent.mHitPosition = this->mReactiveHitPosition;
  viewportEvent.mHitObject = GetReactiveSpace()->mOver;
  viewportEvent.mWorldRay = this->mReactiveRay;
  viewportEvent.mRayStart = this->mReactiveRay.Start;
  viewportEvent.mRayDirection = this->mReactiveRay.Direction;
  viewportEvent.mViewport = this;
  viewportEvent.Handled = false;

  viewportEvent.mCameraViewportCog = mViewportInterface->GetOwner();
}

Widget* ReactiveViewport::HitTest(Vec2 screenPoint, Widget* skip)
{
  Widget* hit = Composite::HitTest(screenPoint, skip);
  if (hit == nullptr)
    hit = Widget::HitTest(screenPoint, skip);
  return hit;
}

//------------------------------------------------------------------ Game Widget
ZilchDefineType(GameWidget, builder, type)
{
}

//******************************************************************************
GameWidget::GameWidget(Composite* composite)
  : Composite(composite)
{
  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
  ConnectThisTo(GetRootWidget(), Events::WidgetUpdate, OnUpdate);
}

//******************************************************************************
GameWidget::~GameWidget()
{

}

//******************************************************************************
void GameWidget::OnDestroy()
{
  // Signal quit, handle will be null if GameWidget is not supposed to quit game
  if (GameSession* game = mGame)
    game->Quit();
  Composite::OnDestroy();
}

//******************************************************************************
bool GameWidget::TakeFocusOverride()
{
  CommandManager::GetInstance()->ClearContext(ZilchTypeId(Space));

  this->HardTakeFocus();
  return true;
}

//******************************************************************************
void GameWidget::OnKeyDown(KeyboardEvent* event)
{
  if (event->Handled)
    return;

  GameSession* game = mGame;
  if (game == nullptr)
    return;

  bool editorMode = game->IsEditorMode();

  // Check for escape for default escape logic
  if (event->Key == Keys::Escape)
  {
    if (!editorMode)
      game->RequestQuit();
  }

  if (event->Key == Keys::F9)
  {
    event->Handled = true;

    // Check for edit in game
    GameSession* gameSession = mGame;
    if (gameSession)
      gameSession->EditSpaces();
  }

  // This is a temporary fix for events bubbling up that don't get handled in the game
  // In general, pressing number keys in the game should NOT change tools, or Ctrl+S should not save
  Keys::Enum key = event->Key;
  if (!editorMode && key != Keys::Escape && !(key >= Keys::F5 && key <= Keys::F12) && !(event->CtrlPressed && event->ShiftPressed))
    event->Handled = true;
  if (event->HandledEventScript)
    event->Handled = true;
}

//******************************************************************************
ReactiveViewport* GameWidget::GetViewportUnder(ReactiveViewport* current)
{
  // We use previous because widgets draw back to front
  Widget* currentWidget = PreviousSibling(current);

  while (currentWidget)
  {
    ReactiveViewport* nextViewport = Type::DynamicCast<ReactiveViewport*>(currentWidget);
    if (nextViewport)
      return nextViewport;

    currentWidget = PreviousSibling(currentWidget);
  }

  return nullptr;
}

//******************************************************************************
void GameWidget::OnGameQuit(GameEvent* gameEvent)
{
  CloseTabContaining(this);
}

//******************************************************************************
void GameWidget::SetGameSession(GameSession* gameSession)
{
  mGame = gameSession;
  ConnectThisTo(gameSession, Events::GameQuit, OnGameQuit);
}

//******************************************************************************
void GameWidget::SaveScreenshot(StringParam filename)
{
  mScreenshotFilename = filename;
  ConnectThisTo(Z::gEngine->has(GraphicsEngine), "UiRenderUpdate", OnUiRenderUpdate);
}

//******************************************************************************
void GameWidget::OnUiRenderUpdate(Event* event)
{
  // Update runs one time to add tasks for a screenshot and then disconnects
  GraphicsEngine* graphics = Z::gEngine->has(GraphicsEngine);
  RenderTasks& renderTasks = *graphics->mRenderTasksBack;
  RenderQueues& renderQueues = *graphics->mRenderQueuesBack;

  FrameBlock& frameBlock = renderQueues.mFrameBlocks.PushBack();
  ViewBlock& viewBlock = renderQueues.mViewBlocks.PushBack();
  frameBlock.mRenderQueues = &renderQueues;

  Vec2 size = GetSize();
  Mat4 translation; translation.Translate(size.x * -0.5f, size.y * -0.5f, 0.0f);
  Mat4 scale; scale.Scale(1.0f, -1.0f, 1.0f);
  viewBlock.mWorldToView = scale * translation;
  BuildOrthographicTransformZero(viewBlock.mViewToPerspective, size.y, size.x / size.y, -1.0f, 1.0f);

  Mat4 apiPerspective;
  Z::gRenderer->BuildOrthographicTransform(apiPerspective, size.y, size.x / size.y, -1.0f, 1.0f);
  viewBlock.mZeroPerspectiveToApiPerspective = apiPerspective * viewBlock.mViewToPerspective.Inverted();

  ColorTransform colorTx = {Vec4(1.0f)};
  Rect clipRect = {0, 0, size.x, size.y};
  RenderUpdate(viewBlock, frameBlock, Mat4::cIdentity, colorTx, clipRect);

  IndexRange& indexRange = viewBlock.mRenderGroupRanges.PushBack();
  indexRange.start = 0;
  indexRange.end = viewBlock.mViewNodes.Size();

  RenderTaskRange& renderTaskRange = renderTasks.mRenderTaskRanges.PushBack();
  renderTaskRange.mFrameBlockIndex = renderQueues.mFrameBlocks.Size() - 1;
  renderTaskRange.mViewBlockIndex = renderQueues.mViewBlocks.Size() - 1;
  renderTaskRange.mTaskIndex = renderTasks.mRenderTaskBuffer.mCurrentIndex;
  renderTaskRange.mTaskCount = 0;

  HandleOf<Texture> textureHandle = Texture::CreateRuntime();
  Texture* texture = textureHandle;
  texture->mFormat = TextureFormat::RGBA8;
  texture->mWidth = (uint)size.x;
  texture->mHeight = (uint)size.y;
  graphics->AddTexture(texture);
  RenderTarget* renderTarget = graphics->GetRenderTarget(texture);

  RenderSettings renderSettings;
  renderSettings.SetColorTarget(renderTarget);
  renderSettings.mBlendSettings[0].SetBlendAlpha();
  renderSettings.mScissorMode = ScissorMode::Enabled;

  renderTasks.mRenderTaskBuffer.AddRenderTaskClearTarget(renderSettings, GetRootWidget()->mClearColor, 0, 0, 0);
  renderTasks.mRenderTaskBuffer.AddRenderTaskRenderPass(renderSettings, 0, "ColorOutput", 0);
  renderTaskRange.mTaskCount = 2;

  graphics->WriteTextureToFile(textureHandle, mScreenshotFilename);

  DisconnectAll(graphics, this);
}

//******************************************************************************
// Sort the viewports for input forwarding
struct ViewportSorter
{
  bool operator()(Widget& left, Widget& right)
  {
    Viewport* leftViewport = Type::DynamicCast<Viewport*>(&left);
    Viewport* rightViewport = Type::DynamicCast<Viewport*>(&right);

    if (leftViewport && rightViewport)
    {
      CameraViewport* leftCameraViewport = (CameraViewport*)leftViewport->mViewportInterface;
      CameraViewport* rightCameraViewport = (CameraViewport*)rightViewport->mViewportInterface;

      // Objects and Widgets are not delay destructed at the same time,
      // so make sure CameraViewports are still valid
      if (leftCameraViewport && rightCameraViewport)
        return leftCameraViewport->mRenderOrder < rightCameraViewport->mRenderOrder;
      else
        return (leftCameraViewport != nullptr);
    }

    // Not both viewports so return that viewports are less
    return (leftViewport != nullptr);
  }
};

//******************************************************************************
void GameWidget::OnUpdate(UpdateEvent* event)
{
  mChildren.Sort(ViewportSorter());
}

} // namespace Zero
