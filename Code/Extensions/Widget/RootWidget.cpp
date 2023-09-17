// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
namespace Zero
{
Array<Widget*> RootWidget::sMouseTrapFocusWidgets;

namespace Interaction
{
const cstr cLocation = "EditorUi/Interaction";
Tweakable(float, MouseHoldTime, 0.1f, cLocation);
Tweakable(float, MouseHoverTime, 0.2f, cLocation);
Tweakable(float, MouseDragDistance, 8.0f, cLocation);
Tweakable(bool, DebugMouseInteraction, false, cLocation);
Tweakable(bool, DebugMouseEvents, false, cLocation);
} // namespace Interaction

namespace RootWidgetUi
{
const cstr cLocation = "EditorUi/";
Tweakable(Vec4, ClearColor, ToFloatColor(ByteColorRGBA(46, 46, 46, 255)), cLocation);
} // namespace RootWidgetUi

ZilchDefineType(RootWidget, builder, type)
{
  ZeroBindEvent(Events::Closing, HandleableEvent);
}

RootWidget::RootWidget() : Composite(NULL)
{
  WidgetManager::GetInstance()->RootWidgets.PushBack(this);

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
  mDragged = false;

  mDebuggerOverlay = new ColorBlock(this);
  mDebuggerOverlay->SetColor(Vec4(0, 0, 0, 0.4f));
  mDebuggerOverlay->SetSize(Pixels(50000, 50000));
  mDebuggerOverlay->SetActive(false);
  mDebuggerText = new Text(this, "DebuggerText");
  mDebuggerText->SetText("Debugging...");
  mDebuggerText->SizeToContents();
  mDebuggerText->SetActive(false);

  ConnectThisTo(Z::gWidgetManager, Events::WidgetUpdate, OnManagerUpdate);

  ConnectThisTo(OsWindow::sInstance, Events::OsResized, OnOsResize);
  ConnectThisTo(OsWindow::sInstance, Events::OsMouseDown, OnOsMouseDown);
  ConnectThisTo(OsWindow::sInstance, Events::OsMouseUp, OnOsMouseUp);
  ConnectThisTo(OsWindow::sInstance, Events::OsMouseMove, OnOsMouseMoved);

  ConnectThisTo(OsWindow::sInstance, Events::OsWindowBorderHitTest, OnOsWindowBorderHitTest);

  ConnectThisTo(OsWindow::sInstance, Events::OsMouseScroll, OnOsMouseScroll);

  ConnectThisTo(OsWindow::sInstance, Events::OsKeyTyped, OnOsKeyTyped);
  ConnectThisTo(OsWindow::sInstance, Events::OsKeyRepeated, OnOsKeyDown);
  ConnectThisTo(OsWindow::sInstance, Events::OsKeyDown, OnOsKeyDown);
  ConnectThisTo(OsWindow::sInstance, Events::OsKeyUp, OnOsKeyUp);

  ConnectThisTo(OsWindow::sInstance, Events::OsFocusGained, OnOsFocusGained);
  ConnectThisTo(OsWindow::sInstance, Events::OsFocusLost, OnOsFocusLost);

  ConnectThisTo(OsWindow::sInstance, Events::OsMouseFileDrop, OnOsMouseDrop);
  ConnectThisTo(OsWindow::sInstance, Events::OsPaint, OnOsPaint);

  ConnectThisTo(OsWindow::sInstance, Events::OsClose, OnClose);

  OsShell* shell = Z::gEngine->has(OsShell);
  ConnectThisTo(shell, Events::Cut, OnCutCopyPaste);
  ConnectThisTo(shell, Events::Copy, OnCutCopyPaste);
  ConnectThisTo(shell, Events::Paste, OnCutCopyPaste);

  ConnectThisTo(Z::gEngine, Events::DebuggerPause, OnDebuggerPause);
  ConnectThisTo(Z::gEngine, Events::DebuggerResume, OnDebuggerResume);

  ConnectThisTo(Z::gEngine->has(TimeSystem), "UiUpdate", OnUiUpdate);
  ConnectThisTo(Z::gEngine->has(GraphicsEngine), "UiRenderUpdate", OnUiRenderUpdate);
}

RootWidget::~RootWidget()
{
  InList<RootWidget>::Unlink(this);
  delete OsWindow::sInstance;
}

void RootWidget::OnUiUpdate(UpdateEvent* event)
{
  if (GetTransformUpdateState() != TransformUpdateState::Updated)
    UpdateTransform();

  MouseUpdate(event->Dt);
}

void LineSegment(StreamedVertexArray& streamedVertices, Vec3Param p0, Vec3Param p1, Vec4Param color)
{
  StreamedVertex start(p0, Vec2::cZero, color);
  StreamedVertex end(p1, Vec2::cZero, color);
  streamedVertices.PushBack(start);
  streamedVertices.PushBack(end);
}

void DrawBoxAround(StreamedVertexArray& streamedVertices, ViewNode& lineNode, Widget* widget, ByteColor color)
{
  WidgetRect screenRect = widget->GetScreenRect();

  Vec3 topLeft = Math::TransformPoint(lineNode.mLocalToView, ToVector3(screenRect.TopLeft()));
  Vec3 topRight = Math::TransformPoint(lineNode.mLocalToView, ToVector3(screenRect.TopRight()));
  Vec3 botLeft = Math::TransformPoint(lineNode.mLocalToView, ToVector3(screenRect.BottomLeft()));
  Vec3 botRight = Math::TransformPoint(lineNode.mLocalToView, ToVector3(screenRect.BottomRight()));

  Vec4 color4 = ToFloatColor(color);

  LineSegment(streamedVertices, topLeft, topRight, color4);
  LineSegment(streamedVertices, topLeft, botLeft, color4);
  LineSegment(streamedVertices, botLeft, botRight, color4);
  LineSegment(streamedVertices, topRight, botRight, color4);
}

void DrawChain(
    StreamedVertexArray& streamedVertices, ViewNode& lineNode, Widget* widget, ByteColor base, ByteColor parents)
{
  if (widget)
  {
    Widget* parent = widget->GetParent();
    while (parent != NULL)
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
  WidgetRect clipRect = {0, 0, mSize.x, mSize.y};

  GraphicsEngine* graphics = Z::gEngine->has(GraphicsEngine);
  RenderTasks& renderTasks = *graphics->mRenderTasksBack;
  RenderQueues& renderQueues = *graphics->mRenderQueuesBack;
  renderQueues.mRenderTasks = &renderTasks;

  FrameBlock& frameBlock = renderQueues.mFrameBlocks.PushBack();
  ViewBlock& viewBlock = renderQueues.mViewBlocks.PushBack();
  frameBlock.mRenderQueues = &renderQueues;

  Mat4 translation;
  translation.Translate(mSize.x * -0.5f, mSize.y * -0.5f, 0.0f);
  Mat4 scale;
  scale.Scale(1.0f, -1.0f, 1.0f);
  viewBlock.mWorldToView = scale * translation;
  BuildOrthographicTransformZero(viewBlock.mViewToPerspective, mSize.y, mSize.x / mSize.y, -1.0f, 1.0f);

  Mat4 apiPerspective;
  Z::gRenderer->BuildOrthographicTransform(apiPerspective, mSize.y, mSize.x / mSize.y, -1.0f, 1.0f);
  viewBlock.mZeroPerspectiveToApiPerspective = apiPerspective * viewBlock.mViewToPerspective.Inverted();

  RenderUpdate(viewBlock, frameBlock, Mat4::cIdentity, colorTx, clipRect);

  // Interaction debug draw
  if (Interaction::DebugMouseInteraction)
  {
    StreamedVertexArray& streamedVertices = frameBlock.mRenderQueues->mStreamedVertices;
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

  HandleOf<RenderTarget> renderTarget =
      Z::gEngine->has(GraphicsEngine)->GetRenderTarget((uint)mSize.x, (uint)mSize.y, TextureFormat::RGBA8);

  GraphicsRenderSettings renderSettings;
  renderSettings.SetColorTarget(renderTarget);
  renderSettings.mBlendSettings[0].SetBlendAlpha();
  renderSettings.mScissorMode = ScissorMode::Enabled;

  BoundType* defaultRenderPass = MetaDatabase::GetInstance()->FindType("ColorOutput");
  ReturnIf(defaultRenderPass == nullptr, , "We expected to have a type defined called ColorOutput");

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

  RenderTaskHelper helper(renderTasks.mRenderTaskBuffer);
  helper.AddRenderTaskClearTarget(renderSettings, mClearColor, 0, 0, 0xFF);
  helper.AddRenderTaskRenderPass(renderSettings, 0, defaultRenderPass->Name, shaderInputsId);

  ScreenViewport viewport = {0, 0, (int)mSize.x, (int)mSize.y};
  helper.AddRenderTaskBackBufferBlit(renderTarget, viewport);

  renderTaskRange.mTaskCount = 3;

  Z::gEngine->has(GraphicsEngine)->ClearRenderTargets();
}

void RootWidget::OnManagerUpdate(UpdateEvent* event)
{
  this->DispatchEvent(Events::WidgetUpdate, event);
}

void RootWidget::UpdateTransform()
{
  Vec2 size = ToVec2(Shell::sInstance->GetClientSize());
  if (mSize != size)
  {
    WidgetListRange children = GetChildren();
    if (!children.Empty())
      children.Front().SetSize(size);
    SetTranslationAndSize(Vec3(0, 0, 0), size);
  }

  mDebuggerOverlay->MoveToFront();
  mDebuggerText->MoveToFront();
  WidgetRect rect = this->GetLocalRect();
  PlaceCenterToRect(rect, mDebuggerText);

  Composite::UpdateTransform();
}

void RootWidget::Refresh()
{
  UpdateTransform();
}

float LargestAxis(Vec2 dragMovemvent)
{
  return Math::Max(Math::Abs(dragMovemvent.x), Math::Abs(dragMovemvent.y));
}

void PrintTreeR(Widget* widget)
{
  if (widget != NULL)
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
  while (aChain != NULL)
  {
    parents.Insert(aChain);
    aChain = aChain->GetParent();
  }

  Widget* bChain = objectB;
  while (bChain != NULL)
  {
    if (parents.Contains(bChain))
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
// the base objects and up the trees to the lowest common ancestor of the
// object.
void SendHierarchyEvents(cstr op,
                         Widget* oldObject,
                         Widget* newObject,
                         Event* outEvent,
                         Event* inEvent,
                         StringParam outEventName,
                         StringParam inEventName,
                         StringParam outHierarchyName,
                         StringParam inHierarchyName,
                         uint flag,
                         uint hierarchyFlag)
{

  // Find the lowest common ancestor this object will
  // will stop the bubble of the hierarchy events so
  Widget* lca = LowestCommonAncestor(newObject, oldObject);

  if (Interaction::DebugMouseInteraction)
  {
    ZPrint("%s %s -> %s , Lca %s\n",
           op,
           oldObject ? oldObject->GetDebugName().c_str() : "None",
           newObject ? newObject->GetDebugName().c_str() : "None",
           lca ? lca->GetDebugName().c_str() : "None");
  }

  // Send the out event on the old object
  if (oldObject)
  {
    oldObject->DispatchBubble(outEventName, outEvent);
    oldObject->mFlags.ClearFlag(flag);
  }

  // Now send the out hierarchy event up the old tree
  // until the lowest common ancestor
  Widget* oldChain = oldObject;
  while (oldChain != lca)
  {
    oldChain->mFlags.ClearFlag(hierarchyFlag);
    oldChain->DispatchEvent(outHierarchyName, outEvent);
    oldChain = oldChain->GetParent();
  }

  // Send the in event on the new object
  if (newObject)
  {
    newObject->DispatchBubble(inEventName, inEvent);
    newObject->mFlags.SetFlag(flag);
  }

  // Now send the out hierarchy event up the new tree
  // until the lowest common ancestor
  Widget* newChain = newObject;
  while (newChain != lca)
  {
    newChain->DispatchEvent(inHierarchyName, inEvent);
    newChain->mFlags.SetFlag(hierarchyFlag);
    newChain = newChain->GetParent();
  }
}

void RootWidget::RootChangeFocus(Widget* newFocus, FocusMode::Type focusMode)
{
  Widget* oldFocus = mFocus;
  MarkAsNeedsUpdate();
  // Do not change it the object is already the focus object
  if (oldFocus != newFocus)
  {
    // Send the Focus to the Hierarchy
    FocusEvent focusEvent(newFocus, oldFocus);

    if (Interaction::DebugMouseInteraction)
      ZPrint("Focus Change %s\n", FocusMode::Names[focusMode]);

    SendHierarchyEvents("Focus",
                        oldFocus,
                        newFocus,
                        &focusEvent,
                        &focusEvent,
                        Events::FocusLost,
                        Events::FocusGained,
                        Events::FocusLostHierarchy,
                        Events::FocusGainedHierarchy,
                        DisplayFlags::Focus,
                        DisplayFlags::FocusHierarchy);

    // Store the current focus object
    mFocus = newFocus;
    mFocusMode = focusMode;
  }
}

void RootWidget::FocusReset()
{
  Widget* focusObject = mFocus;

  FocusEvent focusEvent(focusObject, NULL);

  if (focusObject)
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

void RootWidget::RootSoftTakeFocus(Widget* newFocus)
{
  Widget* oldFocus = mFocus;
  if (oldFocus == NULL || mFocusMode == FocusMode::Soft)
  {
    RootChangeFocus(newFocus, FocusMode::Soft);
  }
}

void RootWidget::RootRemoveFocus(Widget* widget)
{
  if (!widget->HasFocus())
    return;

  // Move up one level
  widget = widget->GetParent();

  // Change the focus mode to soft so any widget can
  // take focus
  mFocusMode = FocusMode::Soft;

  // Find the nearest valid widget
  while (widget)
  {
    if (!widget->mDestroyed)
    {
      widget->SoftTakeFocus();
      return;
    }
    widget = widget->GetParent();
  }
}

void RootWidget::RootCaptureMouse(Widget* widget)
{
  Shell::sInstance->SetMouseCapture(true);
  mCaptured = widget;
  // need to set the down object to the capture widget
  // so that drags are only sent to the captured widget
  mDown = widget;

  if (Interaction::DebugMouseInteraction)
  {
    ZPrint("Mouse Captured ");
    PrintPath(widget);
  }

  // Capture also takes key board focus
  RootChangeFocus(widget, FocusMode::Hard);
}

void RootWidget::RootReleaseMouseCapture(Widget* object)
{
  // If the widget releasing capture is being destroyed then we need to use the
  // handle id to check for equality since the handle would otherwise give a
  // null pointer.
  if (WidgetHandleManager::HandleToId(mCaptured) == object->mId)
  {
    mCaptured = nullptr;
    Shell::sInstance->SetMouseCapture(false);
  }
}

void RootWidget::MouseUpdate(float dt)
{
  // Get the object the mouse is over
  Widget* hoverObject = mOver;

  // Captured objects should intercept everything
  if (Widget* captured = mCaptured)
    hoverObject = captured;

  if (hoverObject)
  {
    mHoverTime += dt;

    MouseEvent mouseEvent;
    mouseEvent.EventMouse = Z::gMouse;
    mouseEvent.Source = hoverObject;
    mouseEvent.Position = Z::gMouse->mClientPosition;

    for (uint i = 0; i < MouseButtons::Size; ++i)
      mouseEvent.mButtonDown[i] = Z::gMouse->mButtonDown[i];

    if (mHoverTime > Interaction::MouseHoverTime)
    {
      mHoverTime = -Math::PositiveMax();
      hoverObject->DispatchBubble(Events::MouseHover, &mouseEvent);
    }

    if (mAnyMouseDown)
    {
      mHoldTime += dt;

      // Mouse is being held down update hold time
      if (mHoldTime > Interaction::MouseHoldTime)
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

void RootWidget::OnCutCopyPaste(ClipboardEvent* event)
{
  if (Widget* focusObject = mFocus)
    focusObject->DispatchBubble(event->EventId, event);
}

void RootWidget::OnOsKeyDown(KeyboardEvent* keyboardEvent)
{
  mNeedsRedraw = true;

  Widget* focusObject = mFocus;

  if (focusObject)
  {
    // Allow higher level logic to block keyboard events
    focusObject->DispatchBubble(Events::KeyPreview, keyboardEvent);

    if (keyboardEvent->Handled)
      return;

    // Send out the general key down
    String eventId = cKeyboardEventsFromState[keyboardEvent->State];
    focusObject->DispatchBubble(eventId, keyboardEvent);
  }
}

void RootWidget::OnOsKeyUp(KeyboardEvent* event)
{
  Widget* focusObject = mFocus;
  if (focusObject)
    focusObject->DispatchBubble(Events::KeyUp, event);
}

void RootWidget::OnOsKeyTyped(KeyboardTextEvent* textEvent)
{
  Widget* focusObject = mFocus;
  if (focusObject)
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

void RootWidget::OnOsWindowBorderHitTest(OsWindowBorderHitTest* event)
{
  Widget* overObject = HitTest(ToVec2(event->ClientPosition), nullptr);
  if (overObject)
    overObject->DispatchBubble(event->EventId, event);
}

void RootWidget::BuildMouseEvent(MouseEvent& event, OsMouseEvent* mouseEvent)
{
  Z::gMouse->mClientPosition = ToVec2(mouseEvent->ClientPosition);

  event.Button = mouseEvent->MouseButton;
  event.ButtonDown = mouseEvent->ButtonDown[mouseEvent->MouseButton];
  event.OsEvent = mouseEvent;
  event.EventMouse = Z::gMouse;
  event.Source = NULL;
  event.Position = ToVec2(mouseEvent->ClientPosition);
  event.Scroll = mouseEvent->ScrollMovement;
  event.Movement = Vec2(0, 0);
  event.AltPressed = mouseEvent->AltPressed;
  event.ShiftPressed = mouseEvent->ShiftPressed;
  event.CtrlPressed = mouseEvent->CtrlPressed;

  for (uint i = 0; i < MouseButtons::Size; ++i)
    event.mButtonDown[i] = Z::gMouse->mButtonDown[i];
}

Widget* RootWidget::UpdateMousePosition(OsMouseEvent* osmouseEvent)
{
  // If the mouse is trapped, return the main viewport here
  if (Shell::sInstance->GetMouseTrap() && !sMouseTrapFocusWidgets.Empty()) {
    return sMouseTrapFocusWidgets.Back();
  }

  Widget* captureObject = mCaptured;
  if (captureObject)
    return captureObject;

  Widget* oldOverObject = mOver;
  // Hit test the widget tree
  Widget* newOverObject = HitTest(ToVec2(osmouseEvent->ClientPosition), NULL);

  // Has the mouse moved over a new object?
  if (newOverObject != oldOverObject)
  {
    MouseEvent mouseEventOut;
    BuildMouseEvent(mouseEventOut, osmouseEvent);
    mouseEventOut.Source = oldOverObject;

    MouseEvent mouseEventIn;
    BuildMouseEvent(mouseEventIn, osmouseEvent);
    mouseEventIn.Source = newOverObject;

    SendHierarchyEvents("Over",
                        oldOverObject,
                        newOverObject,
                        &mouseEventOut,
                        &mouseEventIn,
                        Events::MouseExit,
                        Events::MouseEnter,
                        Events::MouseExitHierarchy,
                        Events::MouseEnterHierarchy,
                        DisplayFlags::MouseOver,
                        DisplayFlags::MouseOverHierarchy);

    mOver = newOverObject;
    mHoverTime = 0;
    mHoldTime = 0;
  }

  return newOverObject;
}

// Get the previous sibling in the tree
Widget* PreviousSibling(Widget* object, bool ignoreInactive)
{
  // If there is no parent, there is no sibling
  Composite* parent = object->GetParent();
  if (!parent)
    return nullptr;

  Widget* prev = (Widget*)WidgetList::Prev(object);

  // If we're ignoring inactive objects, keep looking while the previous sibling
  // is inactive
  while (ignoreInactive && prev != parent->mChildren.End() && !prev->mActive)
    prev = (Widget*)WidgetList::Prev(prev);

  if (prev != parent->mChildren.End())
    return prev;

  return nullptr;
}

// Get the next sibling in the tree
Widget* NextSibling(Widget* object, bool ignoreInactive)
{
  // If there is no parent, there is no sibling
  Composite* parent = object->GetParent();
  if (!parent)
    return nullptr;

  Widget* next = (Widget*)WidgetList::Next(object);

  // If we're ignoring inactive objects, keep looking while the next sibling is
  // inactive
  while (ignoreInactive && next != parent->mChildren.End() && !next->mActive)
    next = (Widget*)WidgetList::Next(next);

  if (next != parent->mChildren.End())
    return next;

  return nullptr;
}

Widget* GetLastChild(Widget* object, bool ignoreInactive)
{
  Composite* c = object->GetSelfAsComposite();
  if (c && !c->mChildren.Empty())
  {
    // If not ignoring inactive objects, return the last child of the last child
    if (!ignoreInactive)
      return GetLastChild(&c->mChildren.Back(), false);
    else
    {
      // Get this object's last child
      Widget* lastChild = &c->mChildren.Back();
      // If it's not active, look for a previous active sibling
      if (!lastChild->mActive)
        lastChild = PreviousSibling(lastChild, true);
      // If there are no active children, return this object
      if (!lastChild)
        return object;
      // Return the last child's last child (will always be active)
      return GetLastChild(lastChild, true);
    }
  }
  else
    return object;
}

Widget* GetPrevious(Widget* object, bool ignoreInactive)
{
  // Get prev sibling if there is one
  Widget* prevSibling = PreviousSibling(object, ignoreInactive);

  // If this is the first child (or first active child), get the parent
  // (parent shouldn't be inactive if this is being called on a child)
  if (!prevSibling)
    return object->GetParent();

  // Return the last child of the sibling
  return GetLastChild(prevSibling, ignoreInactive);
}

Widget* GetNext(Widget* object, bool ignoreInactive)
{
  // If this object has children return the first child (or first active child)
  Composite* c = object->GetSelfAsComposite();
  if (c && !c->mChildren.Empty())
  {
    // If not ignoring inactive objects or the first child is active, return the
    // first child
    if (!ignoreInactive || c->mChildren.Front().mActive)
      return &c->mChildren.Front();
    else
    {
      // Return the first active child
      forRange (Widget& child, c->mChildren.All())
      {
        if (child.mActive)
          return &child;
      }
    }
  }

  // Return next sibling if there is one
  Widget* nextSibling = NextSibling(object, ignoreInactive);
  if (nextSibling)
    return nextSibling;

  // Loop until the root or a parent has a sibling
  Widget* parent = object->GetParent();
  while (parent != nullptr)
  {
    Widget* parentSibling = NextSibling(parent, ignoreInactive);
    if (parentSibling)
      return parentSibling;
    else
      parent = parent->GetParent();
  }

  return nullptr;
}

void FindNextFocus(Widget* object, FocusDirection::Enum direction)
{
  while (object != nullptr)
  {
    object = direction == FocusDirection::Forward ? GetNext(object, true) : GetPrevious(object, true);

    if (object && object->mActive)
    {
      bool focusTaken = object->TryTakeFocus();
      if (focusTaken)
        return;
    }
  }
}

void RootWidget::OnOsMouseMoved(OsMouseEvent* osMouseEvent)
{
  if (Interaction::DebugMouseEvents)
    ZPrint("Mouse Move %d, %d \n", osMouseEvent->ClientPosition.x, osMouseEvent->ClientPosition.y);

  UpdateMouseButtons(osMouseEvent);

  Vec2 mouseMovement = ToVec2(osMouseEvent->Movement);

  Z::gMouse->mClientPosition = ToVec2(osMouseEvent->ClientPosition);
  Z::gMouse->mCursorMovement = mouseMovement;

  if (Interaction::DebugMouseEvents)
    ZPrint("Mouse Moved by %f, %f \n", mouseMovement.x, mouseMovement.y);

  // Find the widget the mouse is over
  // this will send MouseExit / MouseEnter and update
  // the down object
  Widget* targetObject = UpdateMousePosition(osMouseEvent);

  if (targetObject == NULL)
    return;

  MouseDragEvent mouseEvent;
  BuildMouseEvent(mouseEvent, osMouseEvent);
  mouseEvent.StartPosition = mDownPosition;
  mouseEvent.Source = targetObject;
  mouseEvent.Movement = mouseMovement;

  Widget* captured = mCaptured;

  // Send the mouse move
  //   - Note: this will be the captured object if there is one.
  if (targetObject == captured)
    captured->DispatchEvent(Events::MouseMove, &mouseEvent);
  else
    targetObject->DispatchBubble(Events::MouseMove, &mouseEvent);

  mouseEvent.Handled = false;

  // Test for MouseDrag
  Widget* downObject = mDown;

  bool stateBeforeObjectDownDrag = mouseEvent.Handled;

  // Don't send a mouse drag if something is already captured

  if (mAnyMouseDown && downObject && captured == nullptr)
  {
    if (Interaction::DebugMouseInteraction)
    {
      ZPrint("Dragging ");
      PrintPath(downObject);
    }

    // Only activate drag once the drag distance is reached,
    // even when the mouse drags off the object
    mDragMovement += mouseMovement;

    // mDragged prevents sending more than one drag message
    // to a widget
    if (LargestAxis(mDragMovement) >= downObject->mDragDistance && !mDragged)
    {
      if (Z::gMouse->mButtonDown[MouseButtons::Left])
        downObject->DispatchBubble(Events::LeftMouseDrag, &mouseEvent);
      else if (Z::gMouse->mButtonDown[MouseButtons::Right])
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
  if (overObject == NULL)
    return;

  MouseEvent mouseEvent;
  BuildMouseEvent(mouseEvent, osMouseEvent);
  mouseEvent.Source = overObject;

  overObject->DispatchBubble(Events::MouseScroll, &mouseEvent);
}

void RootWidget::UpdateMouseButtons(OsMouseEvent* mouseEvent)
{
  mAnyMouseDown = false;
  for (uint i = 0; i < MouseButtons::Size; ++i)
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

  if (targetObject == NULL)
    return;

  if (Interaction::DebugMouseEvents)
    ZPrint("Mouse %s %s\n", MouseButtons::Names[button], buttonDown ? "IsDown" : "IsUp");

  if (Interaction::DebugMouseInteraction)
  {
    ZPrint("Over ");
    PrintPath(targetObject);
  }

  MouseEvent mouseEvent;
  BuildMouseEvent(mouseEvent, osMouseEvent);
  mouseEvent.ButtonDown = buttonDown;
  mouseEvent.Source = targetObject;

  if (buttonDown)
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
    if (mouseDownObject == targetObject && button < NamedButtonEvents)
      targetObject->DispatchBubble(NamedMouseClick[button], &mouseEvent);

    // Check for double click conditions
    bool buttonsAreTheSame = mLastClickButton == button;
    bool distanceIsSmall = LargestAxis(mouseEvent.Position - mLastClickPosition) < 4.0f;
    float osDoubleClickTime = Os::GetDoubleClickTimeMs() / 1000.0f;
    bool doubleClickTime = mTimeSinceLastClick < osDoubleClickTime;

    mouseEvent.Handled = false;
    if (buttonsAreTheSame && distanceIsSmall && doubleClickTime)
      targetObject->DispatchBubble(Events::DoubleClick, &mouseEvent);

    // Update state for double clicks
    mLastClickButton = button;
    mTimeSinceLastClick = 0;
    mLastClickPosition = mouseEvent.Position;
  }

  // Send out generic mouse down / mouse up
  String mouseEventName = buttonDown ? Events::MouseDown : Events::MouseUp;

  Widget* captured = mCaptured;
  if (captured)
    captured->DispatchEvent(mouseEventName, &mouseEvent);
  else
    targetObject->DispatchBubble(mouseEventName, &mouseEvent);

  // Send out named mouse event
  String* namedMouseEvents = buttonDown ? NamedMouseDown : NamedMouseUp;
  if (button < NamedButtonEvents)
  {
    if (captured)
      captured->DispatchEvent(namedMouseEvents[button], &mouseEvent);
    else
      targetObject->DispatchBubble(namedMouseEvents[button], &mouseEvent);
  }
}

void RootWidget::OnOsMouseDrop(OsMouseDropEvent* mouseDrop)
{
  Widget* targetObject = UpdateMousePosition(mouseDrop);
  if (targetObject == NULL)
    return;

  MouseEvent mouseEvent;
  BuildMouseEvent(mouseEvent, mouseDrop);
  mouseEvent.Source = targetObject;

  targetObject->DispatchBubble(Events::MouseDrop, &mouseEvent);

  MouseFileDropEvent fileDrop(mouseEvent);
  fileDrop.Copy(*mouseDrop);

  targetObject->DispatchBubble(Events::MouseFileDrop, &fileDrop);
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
  if (!event.Handled)
    this->Destroy();
}

void RootWidget::OnDebuggerPause(Event* event)
{
  mDebuggerOverlay->SetActive(true);
  mDebuggerText->SetActive(true);
}

void RootWidget::OnDebuggerResume(Event* event)
{
  // We don't immediately disable the debugger overlay because we want all Os
  // messages to be processed by the overlay before fully resuming (prevents
  // lots of ghost mouse clicks and weird effects)
  ActionSequence* seq = new ActionSequence(this);
  seq->Add(new ActionDelayOnce());
  seq->Add(new CallAction<RootWidget, &RootWidget::OnDebuggerResumeDelay>(this));
}

void RootWidget::OnDebuggerResumeDelay()
{
  mDebuggerOverlay->SetActive(false);
  mDebuggerText->SetActive(false);
}

} // namespace Zero
