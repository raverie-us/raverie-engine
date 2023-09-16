// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

void WidgetFlagCallback(Cog* cog, uint flag, FlagOperation::Enum operation)
{
  if (UiWidget* widget = cog->has(UiWidget))
  {
    if (operation == FlagOperation::Set)
      widget->mFlags.SetFlag(flag);
    else
      widget->mFlags.ClearFlag(flag);
  }
}

ZilchDefineType(UiRootWidget, builder, type)
{
  ZeroBindDocumented();
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(UiWidget);

  // Events
  ZeroBindEvent(Events::UiFocusGainedPreview, UiFocusEvent);
  ZeroBindEvent(Events::UiFocusGained, UiFocusEvent);
  ZeroBindEvent(Events::UiFocusLost, UiFocusEvent);
  ZeroBindEvent(Events::UiFocusLostHierarchy, UiFocusEvent);
  ZeroBindEvent(Events::UiFocusGainedHierarchy, UiFocusEvent);
  ZeroBindEvent(Events::UiFocusReset, UiFocusEvent);

  // Keyboard events
  ZeroBindEvent(Events::HoverKeyPreview, KeyboardEvent);
  ZeroBindEvent(Events::HoverKeyDown, KeyboardEvent);
  ZeroBindEvent(Events::HoverKeyUp, KeyboardEvent);
  ZeroBindEvent(Events::HoverKeyRepeated, KeyboardEvent);

  // ZilchBindFieldProperty(mSnapSize);

  ZilchBindFieldProperty(mMouseHoverTime);
  ZilchBindFieldProperty(mMouseHoldTime);
  ZilchBindFieldProperty(mDepthSeparation);

  ZilchBindGetterSetterProperty(DebugSelected);

  ZilchBindFieldProperty(mDebugMouseInteraction);

  ZilchBindGetterSetter(FocusWidget);
  ZilchBindGetter(MouseOverWidget);
  ZilchBindGetter(MouseDownWidget);

  // Methods
  ZilchBindMethod(Update);
  ZilchBindMethod(Render);
  // ZilchBindMethod(DispatchAt);
}

UiRootWidget::UiRootWidget()
{
  mSnapSize = 1.0f;
  mCurrHoldTime = 0;
  mCurrHoverTime = 0;
  mTimeSinceLastClick = Math::PositiveMax();
  mMouseButtonDownCount = 0;
  mLastClickedButton = (MouseButtons::Enum)(uint)-1;
  mLastClickPosition = Vec2(Math::PositiveMax());
  mIgnoreEvents = false;

  // Rendering settings
  mStencilCount = 0;
  mStencilDrawMode = StencilDrawMode::None;

  BlendSettings blendSettings;
  blendSettings.SetBlendAlpha();

  DepthSettings depthSettings;
  depthSettings.SetDepthRead(TextureCompareFunc::LessEqual);
  depthSettings.SetStencilIncrement();

  mStencilAddSettings.mBlendSettings[0] = blendSettings;
  mStencilAddSettings.mDepthSettings = depthSettings;

  depthSettings.SetStencilDecrement();
  mStencilRemoveSettings.mBlendSettings[0] = blendSettings;
  mStencilRemoveSettings.mDepthSettings = depthSettings;

  depthSettings.SetStencilTestMode(TextureCompareFunc::Equal);
  mStencilTestSettings.mBlendSettings[0] = blendSettings;
  mStencilTestSettings.mDepthSettings = depthSettings;
}

void UiRootWidget::Serialize(Serializer& stream)
{
  UiWidget::Serialize(stream);

  SerializeNameDefault(mMouseHoverTime, 0.1f);
  SerializeNameDefault(mMouseHoldTime, 1.0f);
  SerializeNameDefault(mDepthSeparation, 0.01f);
  SerializeNameDefault(mDebugMouseInteraction, false);
}

void UiRootWidget::Initialize(CogInitializer& initializer)
{
  UiWidget::Initialize(initializer);

  // If we have a Reactive Component, listen for events that come through it
  if (GetOwner()->has(Reactive))
  {
    // Mouse events
    ConnectThisTo(GetOwner(), Events::LeftMouseDown, OnMouseButton);
    ConnectThisTo(GetOwner(), Events::LeftMouseUp, OnMouseButton);
    ConnectThisTo(GetOwner(), Events::MiddleMouseDown, OnMouseButton);
    ConnectThisTo(GetOwner(), Events::MiddleMouseUp, OnMouseButton);
    ConnectThisTo(GetOwner(), Events::RightMouseDown, OnMouseButton);
    ConnectThisTo(GetOwner(), Events::RightMouseUp, OnMouseButton);
    ConnectThisTo(GetOwner(), Events::MouseScroll, OnMouseEvent);
    ConnectThisTo(GetOwner(), Events::MouseMove, OnMouseEvent);
    ConnectThisTo(GetOwner(), Events::MouseUpdate, OnMouseUpdate);

    // We want to the root widget to know when the mouse has exited the entire
    // thing
    ConnectThisTo(GetOwner(), Events::MouseExit, OnMouseEvent);

    // Keyboard events
    ConnectThisTo(Keyboard::Instance, Events::KeyDown, OnKeyboardEvent);
    ConnectThisTo(Keyboard::Instance, Events::KeyRepeated, OnKeyboardEvent);
  }

  // Keyboard events that have bubbled through the widget system.
  ConnectThisTo(GetOwner(), Events::KeyDown, OnWidgetKeyDown);
  ConnectThisTo(GetOwner(), Events::KeyRepeated, OnWidgetKeyDown);
}

void UiRootWidget::Update()
{
  // Until the TransformUpdateState is fully functional, we should always update
  bool alwaysUpdate = true;

  // Update the widget tree if we need to be updated
  if (mTransformUpdateState != UiTransformUpdateState::Updated || alwaysUpdate)
  {
    UiTransformUpdateEvent e;
    e.mRootWidget = this;
    UiWidget::Update(&e);
  }
}

UiWidget* UiRootWidget::CastPoint(Vec2Param worldPoint, UiWidget* ignore, bool interactiveOnly)
{
  // Walk in reverse to hit the most recently created first
  uint size = mOnTopWidgets.Size();
  for (uint i = size - 1; i < size; --i)
  {
    UiWidget* onTopWidget = mOnTopWidgets[i];

    // Temporarily clear OnTop so that the CastPoint function properly processes
    // this widget
    onTopWidget->mFlags.ClearFlag(UiWidgetFlags::OnTop);

    UiWidget* hitWidget = onTopWidget->CastPoint(worldPoint, ignore, interactiveOnly);

    // Re-set the on top flag
    onTopWidget->mFlags.SetFlag(UiWidgetFlags::OnTop);

    if (hitWidget)
      return hitWidget;
  }

  return UiWidget::CastPoint(worldPoint, ignore, interactiveOnly);
}

void UiRootWidget::UpdateMouseTimers(float dt, ViewportMouseEvent* e)
{
  // Widgets could have moved from under the mouse, so update what the mouse is
  // over
  MouseMove(e);

  UiWidget* hoverWidget = mMouseOverWidget;

  if (hoverWidget)
  {
    Cog* hoverCog = hoverWidget->GetOwner();

    // Update the hover timer
    mCurrHoverTime += dt;

    // DebugPrint("Update Mouse Over %0.3f\n", mCurrHoverTime);

    if (mCurrHoverTime >= mMouseHoverTime)
    {
      mCurrHoverTime = -Math::PositiveMax();
      // DebugPrint("Mouse Hover %s\n", hoverCog->mObjectName.c_str());
      hoverCog->DispatchEvent(Events::MouseHover, e);
      hoverCog->DispatchUp(Events::MouseHover, e);
    }

    if (mMouseButtonDownCount > 0)
    {
      mCurrHoldTime += dt;

      if (mCurrHoldTime > mMouseHoldTime)
      {
        mCurrHoldTime = -Math::PositiveMax();
        hoverCog->DispatchEvent(Events::MouseHold, e);
        hoverCog->DispatchUp(Events::MouseHold, e);
      }
    }
    else
    {
      mCurrHoldTime = 0;
    }

    // Mouse Update
    hoverCog->DispatchEvent(Events::MouseUpdate, e);
    hoverCog->DispatchUp(Events::MouseUpdate, e);
  }

  // Update the click timer
  mTimeSinceLastClick += dt;
}

void UiRootWidget::PerformKeyDown(Keys::Enum key)
{
}

void UiRootWidget::PerformKeyUp(Keys::Enum key)
{
}

void SendKeyboardEvent(KeyboardEvent* e, Cog* cog, StringParam previewEventId, cstr eventPreAppend)
{
  e->Handled = false;

  // Allow higher level logic to block keyboard events
  cog->DispatchEvent(previewEventId, e);
  cog->DispatchUp(previewEventId, e);

  if (e->Handled || e->HandledEventScript)
    return;

  // Send out the general key down
  String eventId = cKeyboardEventsFromState[e->State];

  // Used to pre-append "Hover" to the beginning of each event
  if (eventPreAppend)
    eventId = BuildString(eventPreAppend, eventId);

  cog->DispatchEvent(eventId, e);
  cog->DispatchUp(eventId, e);
}

void UiRootWidget::PerformKeyboardEvent(KeyboardEvent* e)
{
  // Keyboard events first go to the widget in focus
  if (UiWidget* focusWidget = mFocusWidget)
  {
    // e->Handled = false;
    SendKeyboardEvent(e, focusWidget->GetOwner(), Events::KeyPreview, nullptr);
  }

  // If the widget in focus doesn't handle the event, we want to send it to the
  // widget the mouse is currently over
  if (e->Handled || e->HandledEventScript)
    return;

  // Pre-append "Hover" to each event so that the widget knows the context of
  // the event
  if (UiWidget* mouseOver = mMouseOverWidget)
    SendKeyboardEvent(e, mouseOver->GetOwner(), Events::HoverKeyPreview, "Hover");
}

void UiRootWidget::PerformMouseMove(Vec2Param newRootPoint)
{
}

void UiRootWidget::PerformMouseDown(MouseButtons::Enum button, Vec2Param rootPoint)
{
  // ViewportMouseEvent e;
  // BuildMouseEvent(rootPoint, &e);
}

void UiRootWidget::PerformMouseUp(MouseButtons::Enum button, Vec2Param rootPoint)
{
}

void UiRootWidget::PerformMouseScroll(Vec2Param rootPoint, Vec2Param scroll)
{
}

void UiRootWidget::BuildMouseEvent(ViewportMouseEvent* e, Vec2Param rootPoint, MouseButtons::Enum button)
{
  // Mouse* mouse = Z::gMouse;
  //
  // e->Button = button;
  // e->EventMouse = mouse;
  //
  ////e->Source = mMouseOverCog;
  //
  // e->mWorldRay = Ray(Vec3(rootPoint), Vec3(0, 0, -1));
  // e->Position = mouse->GetScreenPosition();
  // e->Movement = mouse->GetScreenMovement();
  //
}

void UiRootWidget::PerformMouseEvent(ViewportMouseEvent* e)
{
  // A mouse event has to be associated with the space the Ui is in
  if (e->GetViewport()->GetTargetSpace() != GetSpace())
  {
    // DoNotifyExceptionAssert("")
    return;
  }

  // If the mouse left the root widget, we need to tell the mouse over widget
  // that it has exited
  if (e->EventId == Events::MouseExit)
    MouseOver(e, nullptr);

  // Update what widget the mouse is over
  if (e->EventId == Events::MouseMove)
    MouseMove(e);

  UiWidget* mouseOverWidget = mMouseOverWidget;

  // Send the event to the mouse over object
  if (mouseOverWidget)
  {
    // Set the correct hit object
    e->mHitObject = mouseOverWidget->GetOwner();

    // Forward events
    Cog* mouseOverCog = mouseOverWidget->GetOwner();
    mouseOverCog->DispatchEvent(e->EventId, e);
    mouseOverCog->DispatchUp(e->EventId, e);
  }
}

float GetLargestAxis(Vec2 dragMovemvent)
{
  return Math::Max(Math::Abs(dragMovemvent.x), Math::Abs(dragMovemvent.y));
}

void UiRootWidget::PerformMouseButton(ViewportMouseEvent* e)
{
  UiWidget* mouseOverWidget = mMouseOverWidget;

  // Set the correct hit object
  if (mouseOverWidget)
    e->mHitObject = mouseOverWidget->GetOwner();

  MouseButtons::Enum button = e->Button;

  // Change focus to widgets that were clicked on
  if (e->ButtonDown)
  {
    ++mMouseButtonDownCount;

    if (mouseOverWidget)
    {
      UiWidget* focusWidget = mouseOverWidget;
      do
      {
        if (focusWidget->GetCanTakeFocus())
        {
          SetFocusWidget(focusWidget);
          break;
        }
        else
        {
          focusWidget = focusWidget->mParent;
        }
      } while (focusWidget);

      mMouseDownWidget = mouseOverWidget;
    }
  }
  // If we load a level containing Ui on mouse down, the mouse up event
  // could be sent to this widget
  else if (mMouseDownWidget != nullptr)
  {
    --mMouseButtonDownCount;

    // Send the click event if we're over the same widget the mouse went down on
    if ((UiWidget*)mMouseDownWidget == mouseOverWidget)
    {
      Cog* mouseOverCog = mouseOverWidget->GetOwner();
      String clickEventId = NamedMouseClick[button];
      mouseOverCog->DispatchEvent(clickEventId, e);
      mouseOverCog->DispatchUp(clickEventId, e);

      // Send the DoubleClick event if we're within the time limit
      bool buttonsAreTheSame = (mLastClickedButton == button);
      bool distanceIsSmall = GetLargestAxis(e->Position - mLastClickPosition) < 4.0f;
      float osDoubleClickTime = Os::GetDoubleClickTimeMs() / 1000.0f;
      bool doubleClickTime = mTimeSinceLastClick < osDoubleClickTime;

      if (buttonsAreTheSame && distanceIsSmall && doubleClickTime)
      {
        mouseOverCog->DispatchEvent(Events::DoubleClick, e);
        mouseOverCog->DispatchUp(Events::DoubleClick, e);
      }

      // Update state for double clicks
      mLastClickedButton = button;
      mTimeSinceLastClick = 0;
      mLastClickPosition = e->Position;
    }

    mMouseDownWidget.Clear();
  }

  // Send the event to the mouse over object
  if (mouseOverWidget)
  {
    Cog* mouseOverCog = mouseOverWidget->GetOwner();

    // Send generic mouse down / up
    String genericEventName = e->ButtonDown ? Events::MouseDown : Events::MouseUp;
    mouseOverCog->DispatchEvent(genericEventName, e);
    mouseOverCog->DispatchUp(genericEventName, e);

    // Send specific button events
    mouseOverCog->DispatchEvent(e->EventId, e);
    mouseOverCog->DispatchUp(e->EventId, e);
  }
}

void UiRootWidget::MouseMove(ViewportMouseEvent* e)
{
  // We want to send mouse events to the widget that the mouse is over
  UiWidget* newMouseOver = CastPoint(ToVector2(e->mHitPosition), nullptr, true);
  while (newMouseOver && !newMouseOver->GetInteractive())
    newMouseOver = newMouseOver->mParent;

  MouseOver(e, newMouseOver);
}

void UiRootWidget::MouseOver(ViewportMouseEvent* e, UiWidget* newMouseOver)
{
  UiWidget* oldMouseOver = mMouseOverWidget;

  // If the mouse has moved to a new object, we want to send MouseEnter and Exit
  // events to both object
  if (newMouseOver != oldMouseOver)
  {
    Cog* oldMouseOverCog = (oldMouseOver) ? oldMouseOver->GetOwner() : nullptr;
    Cog* newMouseOverCog = (newMouseOver) ? newMouseOver->GetOwner() : nullptr;

    // ViewportMouseEvent mouseOut(*e);
    // mouseOut.Source = oldMouseOverCog
    // ViewportMouseEvent mouseIn(*e);
    // mouseOut.Source = newMouseOverCog

    cstr op = (mDebugMouseInteraction) ? "Over" : nullptr;

    if (newMouseOver)
      e->mHitObject = newMouseOver->GetOwner();

    SendHierarchyEvents(op,
                        oldMouseOverCog,
                        newMouseOverCog,
                        e,
                        e,
                        Events::MouseExit,
                        Events::MouseEnter,
                        Events::MouseExitHierarchy,
                        Events::MouseEnterHierarchy,
                        UiWidgetFlags::MouseOver,
                        UiWidgetFlags::MouseOverHierarchy,
                        &WidgetFlagCallback);

    mMouseOverWidget = newMouseOver;
    // DebugPrint("Mouse now over %s\n", newMouseOverCog->mObjectName.c_str());
    mCurrHoverTime = 0.0f;
    mCurrHoldTime = 0.0f;
  }
}

void UiRootWidget::DispatchAt(DispatchAtParams& params)
{
  // Widget* hit = mWidget->CastPoint(params.Position, params.Ignore);
  //
  // if(hit)
  //{
  //  if(params.BubbleEvent)
  //    hit->DispatchBubble(params.EventId, params.EventObject);
  //  else
  //    hit->DispatchEvent(params.EventId, params.EventObject);
  //
  //  params.ObjectHit = true;
  //}
}

void UiRootWidget::OnMouseEvent(ViewportMouseEvent* e)
{
  if (mIgnoreEvents)
    return;

  mIgnoreEvents = true;
  PerformMouseEvent(e);
  mIgnoreEvents = false;

  // If a widget connects to a mouse event on the RootWidget, it will get it
  // once through the 'PerformMouseEvent' call, and once through Reactive
  // (which is where the event for this function came from). The Reactive
  // event will then continue and be sent for a second to other connections
  // (which will get it twice). Because of this, we want to terminate
  //
  // This solves widgets getting the events twice, but it causes an external
  // connection wanting to get reactive events from the root widget to not
  // get them. This should be re-assessed later
  e->Terminate();
}

void UiRootWidget::OnMouseButton(ViewportMouseEvent* e)
{
  if (mIgnoreEvents)
    return;

  mIgnoreEvents = true;
  PerformMouseButton(e);
  mIgnoreEvents = false;

  // See the comment above the Terminate call in 'OnMouseEvent'
  e->Terminate();
}

void UiRootWidget::OnMouseUpdate(ViewportMouseEvent* e)
{
  if (mIgnoreEvents)
    return;

  mIgnoreEvents = true;
  UpdateMouseTimers(0.016f, e);
  mIgnoreEvents = false;

  // See the comment above the Terminate call in 'OnMouseEvent'
  e->Terminate();
}

void UiRootWidget::OnKeyboardEvent(KeyboardEvent* e)
{
  if (mIgnoreEvents)
    return;

  // We're connecting for keyboard on the global keyboard, which gets events
  // after the event is dispatched through the old widget system. The GameWidget
  // sets handled to true after going to the Space to ensure that editor
  // shortcuts aren't executed while in game. So, by the time we get this, the
  // event is handled. However, we don't want all events to be immediately
  // handled, so we have to set it back to false. This should be removed by
  // either not connecting to the global keyboard, or with input refactor.
  e->Handled = false;

  mIgnoreEvents = true;
  PerformKeyboardEvent(e);
  mIgnoreEvents = false;

  // See the comment above the Terminate call in 'OnMouseEvent'
  // e->Terminate();

  e->Handled = true;
}

void UiRootWidget::Render(RenderTasksEvent* e, RenderTarget* color, RenderTarget* depth, MaterialBlock* renderPass)
{
  if (e == nullptr || color == nullptr || depth == nullptr || renderPass == nullptr)
  {
    DoNotifyExceptionAssert("Cannot render Widgets", "All parameters must be satisfied.");
    return;
  }

  if (!IsDepthStencilFormat(depth->mTexture->mFormat))
  {
    DoNotifyExceptionAssert("Cannot render Widgets", "Depth target must have stencil (Depth24Stencil8).");
    return;
  }

  // Reset stencil values
  mStencilDrawMode = StencilDrawMode::None;
  mStencilCount = 0;

  Array<CachedFloatingWidget> floatingWidgets;

  // Render all widgets
  Vec4 colorTransform(1);
  RenderWidgets(e, color, depth, renderPass, this, colorTransform, &floatingWidgets);

  // Render all floating widgets
  forRange (CachedFloatingWidget cachedWidget, floatingWidgets.All())
  {
    UiWidget* widget = cachedWidget.first;
    Vec4 cachedColor = cachedWidget.second;

    RenderWidgets(e, color, depth, renderPass, widget, cachedColor, nullptr);
  }

  FlushGraphicals(e, color, depth, renderPass);
}

void UiRootWidget::RenderWidgets(RenderTasksEvent* e,
                                 RenderTarget* color,
                                 RenderTarget* depth,
                                 MaterialBlock* renderPass,
                                 UiWidget* widget,
                                 Vec4Param colorTransform,
                                 Array<CachedFloatingWidget>* floatingWidgets)
{
  // Don't render inactive, destroyed, or editor hidden widgets
  Cog* widgetCog = widget->GetOwner();
  bool spaceInEditMode = GetSpace()->IsEditorMode();
  bool editorHidden = (spaceInEditMode && widgetCog->GetEditorViewportHidden());
  if (!widget->GetActive() || widgetCog->GetMarkedForDestruction() || editorHidden)
    return;

  if (floatingWidgets && widget->GetOnTop())
  {
    floatingWidgets->PushBack(CachedFloatingWidget(widget, colorTransform));
    return;
  }

  // Build color transform
  Vec4 hierarchyColor = colorTransform * widget->mHierarchyColor;
  Vec4 localColor = hierarchyColor * widget->mLocalColor;

  // Set the color on graphicals
  if (Sprite* sprite = widgetCog->has(Sprite))
    sprite->mVertexColor = localColor;
  else if (SpriteText* spriteText = widgetCog->has(SpriteText))
    spriteText->mVertexColor = localColor;

  if (widget->GetVisible())
  {
    // Write out stencil if we're clipping our children
    if (widget->GetClipChildren())
      AddGraphical(e, color, depth, renderPass, widgetCog, StencilDrawMode::Add, 1);

    // Add the widget to be rendered
    AddGraphical(e, color, depth, renderPass, widgetCog, StencilDrawMode::Test, 0);

    // Recurse to all children
    forRange (UiWidget& child, widget->GetChildren())
      RenderWidgets(e, color, depth, renderPass, &child, hierarchyColor, floatingWidgets);

    // Remove the written stencil data
    if (widget->GetClipChildren())
      AddGraphical(e, color, depth, renderPass, widgetCog, StencilDrawMode::Remove, -1);
  }
}

void UiRootWidget::AddGraphical(RenderTasksEvent* e,
                                RenderTarget* color,
                                RenderTarget* depth,
                                MaterialBlock* renderPass,
                                Cog* widgetCog,
                                StencilDrawMode::Enum stencilMode,
                                uint stencilIncrement)
{
  if (Graphical* graphical = widgetCog->has(Graphical))
  {
    if (graphical->GetVisible() == false)
      return;

    // If the stencil mode has changed, we need to commit all graphicals from
    // the last group
    if (mStencilDrawMode != stencilMode)
      FlushGraphicals(e, color, depth, renderPass);

    mStencilDrawMode = stencilMode;
    mStencilCount += stencilIncrement;
    mGraphicals.Add(graphical);
  }
}

void UiRootWidget::FlushGraphicals(RenderTasksEvent* e,
                                   RenderTarget* color,
                                   RenderTarget* depth,
                                   MaterialBlock* renderPass)
{
  if (mGraphicals.GetCount() == 0)
    return;

  if (mStencilDrawMode == StencilDrawMode::Add)
  {
    mStencilAddSettings.SetDepthTarget(depth);
    e->AddRenderTaskRenderPass(mStencilAddSettings, mGraphicals, *renderPass);
  }
  else if (mStencilDrawMode == StencilDrawMode::Remove)
  {
    mStencilRemoveSettings.SetDepthTarget(depth);
    e->AddRenderTaskRenderPass(mStencilRemoveSettings, mGraphicals, *renderPass);
  }
  else if (mStencilDrawMode == StencilDrawMode::Test)
  {
    mStencilTestSettings.SetColorTarget(color);
    mStencilTestSettings.SetDepthTarget(depth);
    mStencilTestSettings.mDepthSettings.mStencilTestValue = mStencilCount;
    e->AddRenderTaskRenderPass(mStencilTestSettings, mGraphicals, *renderPass);
  }

  mGraphicals.Clear();
}

void UiRootWidget::SetFocusWidget(UiWidget* newFocus)
{
  UiWidget* oldFocus = mFocusWidget;

  // Do not change it the object is already the focus object
  if (oldFocus != newFocus)
  {
    Cog* oldFocusCog = (oldFocus) ? oldFocus->GetOwner() : nullptr;
    Cog* newFocusCog = (newFocus) ? newFocus->GetOwner() : nullptr;

    cstr op = (mDebugMouseInteraction) ? "Focus" : nullptr;

    // Send the Focus to the Hierarchy
    UiFocusEvent focusEvent(newFocus, oldFocus);

    SendHierarchyEvents(op,
                        oldFocusCog,
                        newFocusCog,
                        &focusEvent,
                        &focusEvent,
                        Events::UiFocusLost,
                        Events::UiFocusGained,
                        Events::UiFocusLostHierarchy,
                        Events::UiFocusGainedHierarchy,
                        UiWidgetFlags::HasFocus,
                        UiWidgetFlags::HierarchyHasFocus,
                        &WidgetFlagCallback);

    // Store the current focus object
    mFocusWidget = newFocus;
  }
}

UiWidget* UiRootWidget::GetFocusWidget()
{
  return mFocusWidget;
}

UiWidget* UiRootWidget::GetMouseOverWidget()
{
  return mMouseOverWidget;
}

UiWidget* UiRootWidget::GetMouseDownWidget()
{
  return mMouseDownWidget;
}

void UiRootWidget::OnWidgetKeyDown(KeyboardEvent* e)
{
  if (e->Handled || e->HandledEventScript)
    return;

  if (UiWidget* focusWidget = GetFocusWidget())
    focusWidget->TabJump(e);
}

void UiRootWidget::SetDebugSelected(Cog* selected)
{
  mDebugSelectedWidget = selected;
}

Cog* UiRootWidget::GetDebugSelected()
{
  return mDebugSelectedWidget.GetOwner();
}

} // namespace Zero
