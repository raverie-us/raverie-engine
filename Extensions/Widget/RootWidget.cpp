///////////////////////////////////////////////////////////////////////////////
///
/// \file RootWidget.cpp
/// Implementation of the RootWidget class.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
namespace Zero
{

namespace Interaction
{
  const cstr cLocation = "EditorUi/Interaction";
  Tweakable(float, MouseHoldTime, 0.1f, cLocation);
  Tweakable(float, MouseHoverTime, 0.2f, cLocation);
  Tweakable(float, DoubleClickTime, 0.3f, cLocation);
  Tweakable(float, MouseDragDistance, 8.0f, cLocation);
  Tweakable(bool, DebugMouseInteraction, false, cLocation);
  Tweakable(bool, DebugMouseEvents, false, cLocation);
}

namespace RootWidgetUi
{
  const cstr cLocation = "EditorUi/";
  Tweakable(Vec4, ClearColor, ToFloatColor(ByteColorRGBA(46,  46,  46, 255)), cLocation);
}

ZilchDefineType(RootWidget, builder, type)
{
  ZilchBindGetterProperty(OsWindow);

  ZeroBindEvent(Events::Closing, HandleableEvent);
}

RootWidget::RootWidget(OsWindow* osWindow)
  : Composite(NULL)
{
  mRootWidget = this;
  mHoverTime = 0.0f;
  mHoldTime = 0.0f;
  mNeedsRedraw = true;
  mFocusMode = FocusMode::Soft;
  mAnyMouseDown = false;
  mDragMovement = Vec2(0, 0);
  mLastClickButton = (uint)-1;
  mLastClickPosition = Vec2(0, 0);
  mTimeSinceLastClick = 10000;
  mClearColor = RootWidgetUi::ClearColor;
  mOsWindow = osWindow;
  mDragged = false;

  if(Z::EditorDebugFeatures)
  {
    mDebuggerOverlay = new ColorBlock(this);
    mDebuggerOverlay->SetColor(Vec4(0, 0, 0, 0.4f));
    mDebuggerOverlay->SetSize(Pixels(50000, 50000));
    mDebuggerOverlay->SetActive(false);
    mDebuggerText = new Text(this, "DebuggerText");
    mDebuggerText->SetText("Debugging...");
    mDebuggerText->SizeToContents();
    mDebuggerText->SetActive(false);
  }

  ConnectThisTo(Z::gWidgetManager, Events::WidgetUpdate, OnManagerUpdate);

  ConnectThisTo(osWindow, Events::OsResized, OnOsResize);
  ConnectThisTo(osWindow, Events::OsMouseDown, OnOsMouseDown);
  ConnectThisTo(osWindow, Events::OsMouseUp, OnOsMouseUp);
  ConnectThisTo(osWindow, Events::OsMouseMove, OnOsMouseMoved);

  ConnectThisTo(osWindow, Events::OsMouseScroll, OnOsMouseScroll);

  ConnectThisTo(osWindow, Events::OsKeyTyped, OnOsKeyTyped);
  ConnectThisTo(osWindow, Events::OsKeyRepeated, OnOsKeyDown);
  ConnectThisTo(osWindow, Events::OsKeyDown, OnOsKeyDown);
  ConnectThisTo(osWindow, Events::OsKeyUp, OnOsKeyUp);

  ConnectThisTo(osWindow, Events::OsFocusGained, OnOsFocusGained);
  ConnectThisTo(osWindow, Events::OsFocusLost, OnOsFocusLost);

  ConnectThisTo(osWindow, Events::OsMouseFileDrop, OnOsMouseDrop);
  ConnectThisTo(osWindow, Events::OsPaint, OnOsPaint);

  ConnectThisTo(osWindow, Events::OsClose, OnClose);

  ConnectThisTo(Z::gEngine, Events::DebuggerPause, OnDebuggerPause);
  ConnectThisTo(Z::gEngine, Events::DebuggerResume, OnDebuggerResume);

  ConnectThisTo(Z::gEngine->has(TimeSystem), "UiUpdate", OnUiUpdate);
  ConnectThisTo(Z::gEngine->has(GraphicsEngine), "UiRenderUpdate", OnUiRenderUpdate);
}

RootWidget::~RootWidget()
{
  mOsWindow->Destroy();
}

void RootWidget::OnUiUpdate(UpdateEvent* event)
{
  if (GetTransformUpdateState() != TransformUpdateState::Updated)
    UpdateTransform();

  MouseUpdate(event->Dt);
}

void LineSegment(Array<StreamedVertex>& streamedVertices, Vec3Param p0, Vec3Param p1, Vec4Param color)
{
  StreamedVertex start(p0, Vec2::cZero, color);
  StreamedVertex end(p1, Vec2::cZero, color);
  streamedVertices.PushBack(start);
  streamedVertices.PushBack(end);
}

void DrawBoxAround(Array<StreamedVertex>& streamedVertices, ViewNode& lineNode, Widget* widget, ByteColor color)
{
  Rect screenRect = widget->GetScreenRect();

  Vec3 topLeft  = Math::TransformPoint(lineNode.mLocalToView, ToVector3(screenRect.TopLeft()));
  Vec3 topRight = Math::TransformPoint(lineNode.mLocalToView, ToVector3(screenRect.TopRight()));
  Vec3 botLeft  = Math::TransformPoint(lineNode.mLocalToView, ToVector3(screenRect.BottomLeft()));
  Vec3 botRight = Math::TransformPoint(lineNode.mLocalToView, ToVector3(screenRect.BottomRight()));

  Vec4 color4 = ToFloatColor(color);

  LineSegment(streamedVertices, topLeft,  topRight, color4);
  LineSegment(streamedVertices, topLeft,  botLeft , color4);
  LineSegment(streamedVertices, botLeft,  botRight, color4);
  LineSegment(streamedVertices, topRight, botRight, color4);
}

void DrawChain(Array<StreamedVertex>& streamedVertices, ViewNode& lineNode, Widget* widget, ByteColor base, ByteColor parents)
{
  if(widget)
  {
    Widget* parent = widget->GetParent();
    while(parent != NULL)
    {
      DrawBoxAround(streamedVertices, lineNode, parent, parents);
      parent = parent->GetParent();
    }
    DrawBoxAround(streamedVertices, lineNode, widget, base);
  }
}

void RootWidget::OnUiRenderUpdate(Event* event)
{
  ColorTransform colorTx = {Vec4(1, 1, 1, 1)};
  Rect clipRect = {0, 0, mSize.x, mSize.y};

  GraphicsEngine* graphics = Z::gEngine->has(GraphicsEngine);
  RenderTasks& renderTasks = *graphics->mRenderTasksBack;
  RenderQueues& renderQueues = *graphics->mRenderQueuesBack;
  renderQueues.mRenderTasks = &renderTasks;

  FrameBlock& frameBlock = renderQueues.mFrameBlocks.PushBack();
  ViewBlock& viewBlock = renderQueues.mViewBlocks.PushBack();
  frameBlock.mRenderQueues = &renderQueues;

  Mat4 translation; translation.Translate(mSize.x * -0.5f, mSize.y * -0.5f, 0.0f);
  Mat4 scale; scale.Scale(1.0f, -1.0f, 1.0f);
  viewBlock.mWorldToView = scale * translation;
  BuildOrthographicTransformZero(viewBlock.mViewToPerspective, mSize.y, mSize.x / mSize.y, -1.0f, 1.0f);

  Mat4 apiPerspective;
  Z::gRenderer->BuildOrthographicTransform(apiPerspective, mSize.y, mSize.x / mSize.y, -1.0f, 1.0f);
  viewBlock.mZeroPerspectiveToApiPerspective = apiPerspective * viewBlock.mViewToPerspective.Inverted();

  RenderUpdate(viewBlock, frameBlock, Mat4::cIdentity, colorTx, clipRect);

  // Interaction debug draw
  if(Interaction::DebugMouseInteraction)
  {
    Array<StreamedVertex>& streamedVertices = frameBlock.mRenderQueues->mStreamedVertices;
    ViewNode& lineNode = AddRenderNodes(viewBlock, frameBlock, clipRect, TextureManager::Find("White"));
    lineNode.mStreamedVertexType = PrimitiveType::Lines;

    DrawChain(streamedVertices, lineNode, mOver, Color::Red, Color::Firebrick);
    DrawChain(streamedVertices, lineNode, mFocus, Color::Blue, Color::LightBlue);

    lineNode.mStreamedVertexCount = streamedVertices.Size() - lineNode.mStreamedVertexStart;
  }

  IndexRange& indexRange = viewBlock.mRenderGroupRanges.PushBack();
  indexRange.start = 0;
  indexRange.end = viewBlock.mViewNodes.Size();

  RenderTaskRange& renderTaskRange = renderTasks.mRenderTaskRanges.PushBack();
  renderTaskRange.mFrameBlockIndex = renderQueues.mFrameBlocks.Size() - 1;
  renderTaskRange.mViewBlockIndex = renderQueues.mViewBlocks.Size() - 1;
  renderTaskRange.mTaskIndex = renderTasks.mRenderTaskBuffer.mCurrentIndex;
  renderTaskRange.mTaskCount = 0;

  HandleOf<RenderTarget> renderTarget = Z::gEngine->has(GraphicsEngine)->GetRenderTarget((uint)mSize.x, (uint)mSize.y, TextureFormat::RGBA8);

  RenderSettings renderSettings;
  renderSettings.SetColorTarget(renderTarget);
  renderSettings.mBlendSettings[0].SetBlendAlpha();
  renderSettings.mScissorMode = ScissorMode::Enabled;

  BoundType* defaultRenderPass = MetaDatabase::GetInstance()->FindType("ColorOutput");
  ReturnIf(defaultRenderPass == nullptr,, "We expected to have a type defined called ColorOutput");

  HandleOf<MaterialBlock> renderPassHandle = ZilchAllocate(MaterialBlock, defaultRenderPass);
  MaterialBlock& renderPass = renderPassHandle;

  Material* spriteMaterial = MaterialManager::FindOrNull("AlphaSprite");
  uint shaderInputsId = 0;

  {
    Pair<u64, uint> key((u64)spriteMaterial->mResourceId, shaderInputsId);
    IndexRange range = spriteMaterial->AddShaderInputs(renderTasks.mShaderInputs, renderTasks.mShaderInputsVersion);
    renderTasks.mShaderInputRanges.Insert(key, range);
  }
  {
    Pair<u64, uint> key(cFragmentShaderInputsId, shaderInputsId);
    IndexRange range = renderPass.AddShaderInputs(renderTasks.mShaderInputs);
    renderTasks.mShaderInputRanges.Insert(key, range);
  }

  renderTasks.mRenderTaskBuffer.AddRenderTaskClearTarget(renderSettings, mClearColor, 0, 0, 0xFF);
  renderTasks.mRenderTaskBuffer.AddRenderTaskRenderPass(renderSettings, 0, defaultRenderPass->Name, shaderInputsId);

  ScreenViewport viewport = {0, 0, (int)mSize.x, (int)mSize.y};
  renderTasks.mRenderTaskBuffer.AddRenderTaskBackBufferBlit(renderTarget, viewport);

  renderTaskRange.mTaskCount = 3;

  Z::gEngine->has(GraphicsEngine)->ClearRenderTargets();
}

void RootWidget::OnManagerUpdate(UpdateEvent* event)
{
  this->DispatchEvent(Events::WidgetUpdate, event);
}

void RootWidget::UpdateTransform()
{
  Vec2 size = ToVec2(mOsWindow->GetSize());
  if(mSize != size)
  {
    WindowState::Type windowState = GetOsWindow()->GetState();

    // Do not resize when minimized
    if(windowState == WindowState::Minimized)
      return;

    WidgetListRange children = GetChildren();
    if(!children.Empty())
      children.Front().SetSize(size);
    SetTranslationAndSize(Vec3(0, 0, 0), size);
  }

  if(Z::EditorDebugFeatures)
  {
    mDebuggerOverlay->MoveToFront();
    mDebuggerText->MoveToFront();
    Rect rect = this->GetLocalRect();
    PlaceCenterToRect(rect, mDebuggerText);
  }

  Composite::UpdateTransform();
}

void RootWidget::Refresh()
{
  UpdateTransform();
}

float LargestAxis(Vec2 dragMovemvent)
{
  return Math::Max( Math::Abs(dragMovemvent.x), Math::Abs(dragMovemvent.y));
}

void PrintTreeR(Widget* widget)
{
  if(widget != NULL)
  {
    PrintTreeR(widget->mParent);
    ZPrint("%s/", widget->GetDebugName().c_str());
  }
}

void PrintPath(Widget* widget)
{
  PrintTreeR(widget);
  ZPrint("\n");
}

Widget* LowestCommonAncestor(Widget* objectA, Widget* objectB)
{
  HashSet<Widget*> parents;

  Widget* aChain = objectA;
  while(aChain != NULL)
  {
    parents.Insert(aChain);
    aChain = aChain->GetParent();
  }

  Widget* bChain = objectB;
  while(bChain != NULL)
  {
    if(parents.Contains(bChain))
      return bChain;
    bChain = bChain->GetParent();
  }

  return NULL;
}

void RootWidget::OnOsResize(OsWindowEvent* sizeChange)
{
  Refresh();
}

void RootWidget::OnOsPaint(OsWindowEvent* sizeChange)
{
//  DrawContext* context = Z::gGraphics->mDrawContext;
//  mRenderTask->Perform(context, 0);
}

// When events occur on sub objects like mouse enter/exit it is useful to
// have events that are only sent when the mouse leaves the object and all
// children (MouseExitHierarchy). This function sends the correct events to
// the base objects and up the trees to the lowest common ancestor of the object.
void SendHierarchyEvents(cstr op,
  Widget* oldObject, Widget* newObject,
  Event* outEvent, Event* inEvent,
  StringParam outEventName, StringParam inEventName,
  StringParam outHierarchyName, StringParam inHierarchyName,
  uint flag, uint hierarchyFlag)
{

  // Find the lowest common ancestor this object will
  // will stop the bubble of the hierarchy events so
  Widget* lca = LowestCommonAncestor(newObject, oldObject);

  if(Interaction::DebugMouseInteraction)
  {
    ZPrint("%s %s -> %s , Lca %s\n", op,
      oldObject ? oldObject->GetDebugName().c_str() : "None",
      newObject ? newObject->GetDebugName().c_str() : "None",
      lca ? lca->GetDebugName().c_str() : "None" );
  }

  // Send the out event on the old object
  if(oldObject)
  {
    oldObject->DispatchBubble(outEventName, outEvent);
    oldObject->mFlags.ClearFlag(flag);
  }

  // Now send the out hierarchy event up the old tree
  // until the lowest common ancestor
  Widget* oldChain = oldObject;
  while(oldChain != lca)
  {
    oldChain->mFlags.ClearFlag(hierarchyFlag);
    oldChain->DispatchEvent(outHierarchyName, outEvent);
    oldChain = oldChain->GetParent();
  }

  // Send the in event on the new object
  if(newObject)
  {
    newObject->DispatchBubble(inEventName, inEvent);
    newObject->mFlags.SetFlag(flag);
  }

  // Now send the out hierarchy event up the new tree
  // until the lowest common ancestor
  Widget* newChain = newObject;
  while(newChain != lca)
  {
    newChain->DispatchEvent(inHierarchyName, inEvent);
    newChain->mFlags.SetFlag(hierarchyFlag);
    newChain = newChain->GetParent();
  }

}

void RootWidget::RootChangeFocus(Widget* newFocus, FocusMode::Type focusMode)
{
  Widget* oldFocus = mFocus;

  //Do not change it the object is already the focus object
  if(oldFocus != newFocus)
  {
    // Send the Focus to the Hierarchy
    FocusEvent focusEvent(newFocus, oldFocus);

    if(Interaction::DebugMouseInteraction)
      ZPrint("Focus Change %s\n", FocusMode::Names[focusMode]);


    SendHierarchyEvents("Focus", oldFocus, newFocus, &focusEvent, &focusEvent,
      Events::FocusLost, Events::FocusGained,
      Events::FocusLostHierarchy, Events::FocusGainedHierarchy,
      DisplayFlags::Focus, DisplayFlags::FocusHierarchy);

    // Store the current focus object
    mFocus = newFocus;
    mFocusMode = focusMode;
  }
}

void RootWidget::FocusReset()
{
  Widget* focusObject = mFocus;

  FocusEvent focusEvent(focusObject, NULL);

  if(focusObject)
    focusObject->DispatchBubble(Events::FocusReset, &focusEvent);
}

void RootWidget::ResetHover()
{
  mHoverTime = 0;
}

Widget* RootWidget::GetFocusObject()
{
  return mFocus;
}

OsWindow* RootWidget::GetOsWindow()
{
  return mOsWindow;
}

void RootWidget::RootSoftTakeFocus(Widget* newFocus)
{
  Widget* oldFocus = mFocus;
  if(oldFocus == NULL || mFocusMode == FocusMode::Soft)
  {
    RootChangeFocus(newFocus, FocusMode::Soft);
  }
}

void RootWidget::RootRemoveFocus(Widget* widget)
{
  if(!widget->HasFocus())
    return;

  // Move up one level
  widget = widget->GetParent();

  // Change the focus mode to soft so any widget can
  // take focus
  mFocusMode = FocusMode::Soft;

  // Find the nearest valid widget
  while(widget)
  {
    if(!widget->mDestroyed)
    {
      widget->SoftTakeFocus();
      return;
    }
    widget = widget->GetParent();
  }
}

void RootWidget::RootCaptureMouse(Widget* widget)
{
  mOsWindow->SetMouseCapture(true);
  mCaptured = widget;
  //need to set the down object to the capture widget
  //so that drags are only sent to the captured widget
  mDown = widget;

  if(Interaction::DebugMouseInteraction)
  {
    ZPrint("Mouse Captured ");
    PrintPath(widget);
  }

  //Capture also takes key board focus
  RootChangeFocus(widget, FocusMode::Hard);
}

void RootWidget::RootReleaseMouseCapture(Widget* object)
{
  mCaptured = NULL;
  mOsWindow->SetMouseCapture(false);
}

void RootWidget::MouseUpdate(float dt)
{
  // Not on this window
  if(Z::gMouse->mActiveWindow != this->mOsWindow)
    return;

  // Get the object the mouse is over
  Widget* hoverObject = mOver;

  // Captured objects should intercept everything
  if(Widget* captured = mCaptured)
    hoverObject = captured;

  if(hoverObject)
  {
    mHoverTime += dt;

    MouseEvent mouseEvent;
    mouseEvent.EventMouse = Z::gMouse;
    mouseEvent.Source = hoverObject;
    mouseEvent.Position = Z::gMouse->mClientPosition;

    for(uint i=0;i<MouseButtons::Size;++i)
      mouseEvent.mButtonDown[i] = Z::gMouse->mButtonDown[i];

    if(mHoverTime > Interaction::MouseHoverTime)
    {
      mHoverTime = -Math::PositiveMax();
      hoverObject->DispatchBubble(Events::MouseHover, &mouseEvent);
    }

    if(mAnyMouseDown)
    {
      mHoldTime += dt;

      // Mouse is being held down update hold time
      if(mHoldTime > Interaction::MouseHoldTime)
      {
        // Reset hold time
        mHoldTime = -Math::PositiveMax();
        hoverObject->DispatchBubble(Events::MouseHold, &mouseEvent);
      }
    }
    else
    {
      mHoldTime = 0;
    }

    hoverObject->DispatchBubble(Events::MouseUpdate, &mouseEvent);
  }

  mTimeSinceLastClick += dt;
}

void RootWidget::OnOsKeyDown(KeyboardEvent* keyboardEvent)
{
  mNeedsRedraw = true;

  Widget* focusObject = mFocus;

  if(focusObject)
  {
    // Allow higher level logic to block keyboard events
    focusObject->DispatchBubble(Events::KeyPreview, keyboardEvent);

    if(keyboardEvent->Handled)
      return;

    // Send out the general key down
    String eventId = cKeyboardEventsFromState[keyboardEvent->State];
    focusObject->DispatchBubble(eventId, keyboardEvent);
  }
}

void RootWidget::OnOsKeyUp(KeyboardEvent* event)
{
  Widget* focusObject = mFocus;
  if(focusObject)
    focusObject->DispatchBubble(Events::KeyUp, event);
}

void RootWidget::OnOsKeyTyped(KeyboardTextEvent* textEvent)
{
  Widget* focusObject = mFocus;
  if(focusObject)
    focusObject->DispatchBubble(Events::TextTyped, textEvent);
}

void RootWidget::OnOsMouseDown(OsMouseEvent* mouseEvent)
{
  OnOsMouseButton(mouseEvent, true);
}

void RootWidget::OnOsMouseUp(OsMouseEvent* mouseEvent)
{
  OnOsMouseButton(mouseEvent, false);
}

void RootWidget::BuildMouseEvent(MouseEvent& event, OsMouseEvent* mouseEvent)
{
  Z::gMouse->mActiveWindow = mOsWindow;
  Z::gMouse->mClientPosition = ToVec2(mouseEvent->ClientPosition);

  event.Button = mouseEvent->MouseButton;
  event.ButtonDown = mouseEvent->ButtonDown[mouseEvent->MouseButton];
  event.OsEvent = mouseEvent;
  event.EventMouse = Z::gMouse;
  event.Source = NULL;
  event.Position = ToVec2(mouseEvent->ClientPosition);
  event.Scroll = mouseEvent->ScrollMovement;
  event.Movement = Vec2(0,0);
  event.AltPressed = mouseEvent->AltPressed;
  event.ShiftPressed = mouseEvent->ShiftPressed;
  event.CtrlPressed = mouseEvent->CtrlPressed;

  for(uint i = 0; i < MouseButtons::Size; ++i)
    event.mButtonDown[i] = Z::gMouse->mButtonDown[i];
}


Widget* RootWidget::UpdateMousePosition(OsMouseEvent* osmouseEvent)
{
  Widget* captureObject = mCaptured;
  if(captureObject)
    return captureObject;

  Widget* oldOverObject = mOver;
  // Hit test the widget tree
  Widget* newOverObject = HitTest(ToVec2(osmouseEvent->ClientPosition), NULL);

  // Has the mouse moved over a new object?
  if(newOverObject != oldOverObject)
  {
    MouseEvent mouseEventOut;
    BuildMouseEvent(mouseEventOut, osmouseEvent);
    mouseEventOut.Source = oldOverObject;

    MouseEvent mouseEventIn;
    BuildMouseEvent(mouseEventIn, osmouseEvent);
    mouseEventIn.Source = newOverObject;

    SendHierarchyEvents("Over",
      oldOverObject, newOverObject,
      &mouseEventOut, &mouseEventIn,
      Events::MouseExit, Events::MouseEnter,
      Events::MouseExitHierarchy, Events::MouseEnterHierarchy,
      DisplayFlags::MouseOver, DisplayFlags::MouseOverHierarchy);

    mOver = newOverObject;
    mHoverTime = 0;
    mHoldTime = 0;
  }

  return newOverObject;
}

// Get the previous sibling in the tree
Widget* PreviousSibling(Widget* object)
{
  Composite* parent = object->GetParent();
  Widget* prev = (Widget*)WidgetList::Prev(object);
  if(parent && prev != parent->mChildren.End())
    return prev;
  return NULL;
}

// Get the next sibling in the tree
Widget* NextSibling(Widget* object)
{
  Composite* parent = object->GetParent();
  Widget* next = (Widget*)WidgetList::Next(object);
  if(parent && next != parent->mChildren.End())
    return next;
  return NULL;
}

Widget* GetLastChild(Widget* object)
{
  Composite* c = object->GetSelfAsComposite();
  if(c && !c->mChildren.Empty())
    return GetLastChild(&c->mChildren.Back());
  else
    return object;
}

Widget* GetPrevious(Widget* object)
{
  // get prev sibling if there is one
  Widget* prevSibling = PreviousSibling(object);
  Widget* parent = object->GetParent();
  // If this is first node of a child it is previous node
  if(prevSibling == NULL)
    return object->GetParent();

  // return the last child of the sibling
  return GetLastChild(prevSibling);
}

Widget* GetNext(Widget* object)
{
  // If this object has children return the first child
  Composite* c = object->GetSelfAsComposite();
  if(c && !c->mChildren.Empty())
    return &c->mChildren.Front();

  // Return next sibling if there is one
  Widget* nextSibling = NextSibling(object);
  if(nextSibling)
    return nextSibling;

  // Loop until the root or a parent has a
  // sibling
  Widget* parent = object->GetParent();
  while(parent != NULL)
  {
    Widget* parentSibling = NextSibling(parent);
    if(parentSibling)
      return parentSibling;
    else
      parent = parent->GetParent();
  }
  return NULL;
}

void FindNextFocus(Widget* object, FocusDirection::Enum direction)
{
  while(object != NULL)
  {
    object = direction == FocusDirection::Forward ? GetNext(object) : GetPrevious(object);

    if(object && object->mActive)
    {
      bool focusTaken = object->TryTakeFocus();
      if(focusTaken)
        return;
    }
  }
}

void RootWidget::OnOsMouseMoved(OsMouseEvent* osMouseEvent)
{
  if(Interaction::DebugMouseEvents)
    ZPrint("Mouse Move %d, %d \n", osMouseEvent->ClientPosition.x, osMouseEvent->ClientPosition.y);

  UpdateMouseButtons(osMouseEvent);

  Vec2 mouseMovement = ToVec2(osMouseEvent->ClientPosition) - Z::gMouse->mClientPosition;
  Z::gMouse->mClientPosition = ToVec2(osMouseEvent->ClientPosition);
  Z::gMouse->mCursorMovement = mouseMovement;

  if(Interaction::DebugMouseEvents)
    ZPrint("Mouse Moved by %f, %f \n", mouseMovement.x, mouseMovement.y);

  // We must update the mScreenPostion above before exiting out
  // Normally we ignore mouse movements due to the 'mouse trapped' feature (moving to center over and over)
  // but we need to at least update the mouse's screen position (which should have moved to the center)
  if(osMouseEvent->IsTrapMoveBack)
    return;

  // Find the widget the mouse is over
  // this will send MouseExit / MouseEnter and update
  // the down object
  Widget* targetObject = UpdateMousePosition(osMouseEvent);

  if(targetObject == NULL)
    return;

  MouseDragEvent mouseEvent;
  BuildMouseEvent(mouseEvent, osMouseEvent);
  mouseEvent.StartPosition = mDownPosition;
  mouseEvent.Source = targetObject;
  mouseEvent.Movement = mouseMovement;

  Widget* captured = mCaptured;

  // Send the mouse move
  //   - Note: this will be the captured object if there is one.
  if(targetObject == captured)
    captured->DispatchEvent(Events::MouseMove, &mouseEvent);
  else
    targetObject->DispatchBubble(Events::MouseMove, &mouseEvent);

  mouseEvent.Handled = false;

  // Test for MouseDrag
  Widget* downObject = mDown;

  bool stateBeforeObjectDownDrag = mouseEvent.Handled;

  // Don't send a mouse drag if something is already captured
  
  if(mAnyMouseDown && downObject && captured == nullptr)
  {
    if(Interaction::DebugMouseInteraction)
    {
      ZPrint("Dragging ");
      PrintPath(downObject);
    }

    // Only activate drag once the drag distance is reached,
    // even when the mouse drags off the object
    mDragMovement += mouseMovement;

    // mDragged prevents sending more than one drag message
    // to a widget
    if(LargestAxis(mDragMovement) >= downObject->mDragDistance && !mDragged)
    {
      if(Z::gMouse->mButtonDown[MouseButtons::Left])
        downObject->DispatchBubble(Events::LeftMouseDrag, &mouseEvent);
      else if(Z::gMouse->mButtonDown[MouseButtons::Right])
        downObject->DispatchBubble(Events::RightMouseDrag, &mouseEvent);

      mDragged = true;
    }
  }
  else
  {
    // Mouse is no longer down stop dragging
    mDragMovement = Vec2(0, 0);
    mDown = NULL;
    mDragged = false;
  }
}

void RootWidget::OnOsMouseScroll(OsMouseEvent* osMouseEvent)
{
  UpdateMouseButtons(osMouseEvent);
  Widget* overObject = UpdateMousePosition(osMouseEvent);
  if(overObject == NULL)
    return;

  MouseEvent mouseEvent;
  BuildMouseEvent(mouseEvent, osMouseEvent);
  mouseEvent.Source = overObject;

  overObject->DispatchBubble(Events::MouseScroll, &mouseEvent);
}

void RootWidget::UpdateMouseButtons(OsMouseEvent* mouseEvent)
{
  mAnyMouseDown = false;
  for(uint i = 0; i < MouseButtons::Size; ++i)
  {
    Z::gMouse->mButtonDown[i] = mouseEvent->ButtonDown[i];
    mAnyMouseDown = mAnyMouseDown || Z::gMouse->mButtonDown[i];
  }
}

void RootWidget::OnOsMouseButton(OsMouseEvent* osMouseEvent, bool buttonDown)
{
  UpdateMouseButtons(osMouseEvent);

  uint button = osMouseEvent->MouseButton;

  Widget* targetObject = UpdateMousePosition(osMouseEvent);

  if(targetObject == NULL)
    return;

  if(Interaction::DebugMouseEvents)
    ZPrint("Mouse %s %s\n", MouseButtons::Names[button], buttonDown ? "IsDown" : "IsUp");

  if(Interaction::DebugMouseInteraction)
  {
    ZPrint("Over ");
    PrintPath(targetObject);
  }

  MouseEvent mouseEvent;
  BuildMouseEvent(mouseEvent, osMouseEvent);
  mouseEvent.ButtonDown = buttonDown;
  mouseEvent.Source = targetObject;

  if(buttonDown)
  {
    // Change focus to clicked widget
    RootChangeFocus(targetObject, targetObject->mTakeFocusMode);

    // Update for dragging
    mDown = targetObject;
    mDownPosition = ToVec2(osMouseEvent->ClientPosition);
    mDragged = false;
  }
  else
  {
    // A click is generated only if the mouse goes
    // down and up on the same object
    Widget* mouseDownObject = mDown;
    if(mouseDownObject == targetObject && button < NamedButtonEvents)
      targetObject->DispatchBubble(NamedMouseClick[button], &mouseEvent);

    // Check for double click conditions
    bool buttonsAreTheSame = mLastClickButton == button;
    bool distanceIsSmall = LargestAxis(mouseEvent.Position - mLastClickPosition) < 4.0f;
    bool doubleClickTime = mTimeSinceLastClick < Interaction::DoubleClickTime;

    mouseEvent.Handled = false;
    if(buttonsAreTheSame && distanceIsSmall && doubleClickTime)
      targetObject->DispatchBubble(Events::DoubleClick, &mouseEvent);

    // Update state for double clicks
    mLastClickButton = button;
    mTimeSinceLastClick = 0;
    mLastClickPosition = mouseEvent.Position;
  }

  // Send out generic mouse down / mouse up
  String mouseEventName = buttonDown ? Events::MouseDown : Events::MouseUp;

  Widget* captured = mCaptured;
  if(captured)
    captured->DispatchEvent(mouseEventName, &mouseEvent);
  else
    targetObject->DispatchBubble(mouseEventName, &mouseEvent);

  // Send out named mouse event
  String* namedMouseEvents = buttonDown ? NamedMouseDown : NamedMouseUp;
  if(button < NamedButtonEvents)
  {
    if(captured)
      captured->DispatchEvent(namedMouseEvents[button], &mouseEvent);
    else
      targetObject->DispatchBubble(namedMouseEvents[button], &mouseEvent);
  }

}

void RootWidget::OnOsMouseDrop(OsMouseDropEvent* mouseDrop)
{
  Widget* targetObject = UpdateMousePosition(mouseDrop);
  if(targetObject == NULL)
    return;

  MouseEvent mouseEvent;
  BuildMouseEvent(mouseEvent, mouseDrop);
  mouseEvent.Source = targetObject;

  targetObject->DispatchBubble(Events::MouseDrop, &mouseEvent);
}

Composite* RootWidget::GetPopUp()
{
  return this;
}

void RootWidget::OnOsFocusGained(OsWindowEvent* event)
{
  Widget* waitingFocus = mFocusWaiting;
  RootChangeFocus(waitingFocus, FocusMode::Hard);
}

void RootWidget::OnOsFocusLost(OsWindowEvent* event)
{
  mFocusWaiting = mFocus;
  RootChangeFocus(NULL, FocusMode::Hard);
}

void RootWidget::OnClose(OsWindowEvent* windowEvent)
{
  HandleableEvent event;
  this->DispatchEvent(Events::Closing, &event);
  if(!event.Handled)
    this->Destroy();
}

void RootWidget::OnDebuggerPause(Event* event)
{
  mDebuggerOverlay->SetActive(true);
  mDebuggerText->SetActive(true);
}

void RootWidget::OnDebuggerResume(Event* event)
{
  // We don't immediately disable the debugger overlay because we want all Os messages
  // to be processed by the overlay before fully resuming (prevents lots of ghost mouse clicks and weird effects)
  ActionSequence* seq = new ActionSequence(this);
  seq->Add(new ActionDelayOnce());
  seq->Add(new CallAction<RootWidget, &RootWidget::OnDebuggerResumeDelay>(this));
}

void RootWidget::OnDebuggerResumeDelay()
{
  mDebuggerOverlay->SetActive(false);
  mDebuggerText->SetActive(false);
}

}//namespace Zero
