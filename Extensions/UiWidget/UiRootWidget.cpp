///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//******************************************************************************
void WidgetFlagCallback(Cog* cog, uint flag, FlagOperation::Enum operation)
{
  if(UiWidget* widget = cog->has(UiWidget))
  {
    if(operation == FlagOperation::Set)
      widget->mFlags.SetFlag(flag);
    else
      widget->mFlags.ClearFlag(flag);
  }
}

//------------------------------------------------------------------ Root Widget
//******************************************************************************
ZilchDefineType(UiRootWidget, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(UiWidget);

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

  ZilchBindFieldProperty(mSnapSize);

  ZilchBindFieldProperty(mMouseHoverTime);
  ZilchBindFieldProperty(mMouseHoldTime);
  ZilchBindFieldProperty(mDoubleClickTime);
  ZilchBindFieldProperty(mDepthSeparation);

  ZilchBindGetterSetterProperty(DebugSelected);

  ZilchBindFieldProperty(mDebugMouseInteraction);
  ZilchBindFieldProperty(mAlwaysUpdate);

  // Methods
  ZilchBindMethod(Update);
  //ZilchBindMethod(DispatchAt);
}

//******************************************************************************
UiRootWidget::UiRootWidget()
{
  mCurrHoldTime = 0;
  mCurrHoverTime = 0;
  mTimeSinceLastClick = Math::PositiveMax();
  mMouseButtonDownCount = 0;
  mLastClickedButton = (MouseButtons::Enum)(uint)-1;
  mLastClickPosition = Vec2(Math::PositiveMax());
}

//******************************************************************************
void UiRootWidget::Serialize(Serializer& stream)
{
  SerializeNameDefault(mMouseHoverTime, 0.1f);
  SerializeNameDefault(mMouseHoldTime, 1.0f);
  SerializeNameDefault(mDoubleClickTime, 0.3f);
  SerializeNameDefault(mDepthSeparation, 0.01f);
  SerializeNameDefault(mDebugMouseInteraction, false);
  SerializeNameDefault(mSnapSize, 1.0f);
  SerializeNameDefault(mAlwaysUpdate, false);
}

//******************************************************************************
void UiRootWidget::Initialize(CogInitializer& initializer)
{
  mWidget = GetOwner()->has(UiWidget);
}

//******************************************************************************
void UiRootWidget::Update()
{
  // Update the widget tree if we need to be updated
  if(mWidget->mTransformUpdateState != UiTransformUpdateState::Updated || mAlwaysUpdate)
  {
    UiTransformUpdateEvent e;
    e.mRootWidget = this;
    e.mAlwaysUpdate = mAlwaysUpdate;
    mWidget->UpdateTransform(&e);
  }
}

//******************************************************************************
void UiRootWidget::UpdateMouseTimers(float dt, ViewportMouseEvent* e)
{
  // Widgets could have moved from under the mouse, so update what the mouse is over
  MouseMove(e);

  UiWidget* hoverWidget = mMouseOverWidget;

  if(hoverWidget)
  {
    Cog* hoverCog = hoverWidget->GetOwner();

    // Update the hover timer
    mCurrHoverTime += dt;

    //DebugPrint("Update Mouse Over %0.3f\n", mCurrHoverTime);

    if(mCurrHoverTime >= mMouseHoverTime)
    {
      mCurrHoverTime = -Math::PositiveMax();
      //DebugPrint("Mouse Hover %s\n", hoverCog->mObjectName.c_str());
      hoverCog->DispatchEvent(Events::MouseHover, e);
      hoverCog->DispatchUp(Events::MouseHover, e);
    }

    if(mMouseButtonDownCount > 0)
    {
      mCurrHoldTime += dt;

      if(mCurrHoldTime > mMouseHoldTime)
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

//******************************************************************************
void UiRootWidget::PerformKeyDown(Keys::Enum key)
{

}

//******************************************************************************
void UiRootWidget::PerformKeyUp(Keys::Enum key)
{

}

//******************************************************************************
void SendKeyboardEvent(KeyboardEvent* e, Cog* cog, StringParam previewEventId,
                       cstr eventPreAppend)
{
  e->Handled = false;

  // Allow higher level logic to block keyboard events
  cog->DispatchEvent(previewEventId, e);
  cog->DispatchUp(previewEventId, e);

  if(e->Handled || e->HandledEventScript)
    return;

  // Send out the general key down
  String eventId = cKeyboardEventsFromState[e->State];

  // Used to pre-append "Hover" to the beginning of each event
  if(eventPreAppend)
    eventId = BuildString(eventPreAppend, eventId);

  cog->DispatchEvent(eventId, e);
  cog->DispatchUp(eventId, e);
}

//******************************************************************************
void UiRootWidget::PerformKeyboardEvent(KeyboardEvent* e)
{
  // Keyboard events first go to the widget in focus
  if(UiWidget* focusWidget = mFocusWidget)
  {
    //e->Handled = false;
    SendKeyboardEvent(e, focusWidget->GetOwner(), Events::KeyPreview, nullptr);
  }

  // If the widget in focus doesn't handle the event, we want to send it to the widget
  // the mouse is currently over
  if(e->Handled || e->HandledEventScript)
    return;

  // Pre-append "Hover" to each event so that the widget knows the context of the event
  if(UiWidget* mouseOver = mMouseOverWidget)
    SendKeyboardEvent(e, mouseOver->GetOwner(), Events::HoverKeyPreview, "Hover");
}

//******************************************************************************
void UiRootWidget::PerformMouseMove(Vec2Param newRootPoint)
{

}

//******************************************************************************
void UiRootWidget::PerformMouseDown(MouseButtons::Enum button, Vec2Param rootPoint)
{
  //ViewportMouseEvent e;
  //BuildMouseEvent(rootPoint, &e);
}

//******************************************************************************
void UiRootWidget::PerformMouseUp(MouseButtons::Enum button, Vec2Param rootPoint)
{

}

//******************************************************************************
void UiRootWidget::PerformMouseScroll(Vec2Param rootPoint, Vec2Param scroll)
{

}

//******************************************************************************
void UiRootWidget::BuildMouseEvent(ViewportMouseEvent* e, Vec2Param rootPoint,
                                   MouseButtons::Enum button)
{
  //Mouse* mouse = Z::gMouse;
  //
  //e->Button = button;
  //e->EventMouse = mouse;
  //
  ////e->Source = mMouseOverCog;
  //
  //e->mWorldRay = Ray(Vec3(rootPoint), Vec3(0, 0, -1));
  //e->Position = mouse->GetScreenPosition();
  //e->Movement = mouse->GetScreenMovement();
  //
}

//******************************************************************************
void UiRootWidget::PerformMouseEvent(ViewportMouseEvent* e)
{
  // A mouse event has to be associated with the space the Ui is in
  if(e->GetViewport()->GetTargetSpace() != GetSpace())
  {
    //DoNotifyExceptionAssert("")
    return;
  }

  // If the mouse left the root widget, we need to tell the mouse over widget that it has exited
  if(e->EventId == Events::MouseExit)
    MouseOver(e, nullptr);

  // Update what widget the mouse is over
  if(e->EventId == Events::MouseMove)
    MouseMove(e);

  UiWidget* mouseOverWidget = mMouseOverWidget;

  // Send the event to the mouse over object
  if(mouseOverWidget)
  {
    // Set the correct hit object
    e->mHitObject = mouseOverWidget->GetOwner();

    // Forward events
    Cog* mouseOverCog = mouseOverWidget->GetOwner();
    mouseOverCog->DispatchEvent(e->EventId, e);
    mouseOverCog->DispatchUp(e->EventId, e);
  }
}

//******************************************************************************
float GetLargestAxis(Vec2 dragMovemvent)
{
  return Math::Max(Math::Abs(dragMovemvent.x), Math::Abs(dragMovemvent.y));
}

//******************************************************************************
void UiRootWidget::PerformMouseButton(ViewportMouseEvent* e)
{
  UiWidget* mouseOverWidget = mMouseOverWidget;

  // Set the correct hit object
  if(mouseOverWidget)
    e->mHitObject = mouseOverWidget->GetOwner();

  MouseButtons::Enum button = e->Button;

  // Change focus to widgets that were clicked on
  if(e->ButtonDown)
  {
    ++mMouseButtonDownCount;
    if(mouseOverWidget)
    {
      if (mouseOverWidget->GetCanTakeFocus())
        RootChangeFocus(mouseOverWidget);
      mMouseDownWidget = mouseOverWidget;
    }
  }
  // If we load a level containing Ui on mouse down, the mouse up event
  // could be sent to this widget
  else if(mMouseDownWidget != nullptr)
  {
    --mMouseButtonDownCount;

    // Send the click event if we're over the same widget the mouse went down on
    if((UiWidget*)mMouseDownWidget == mouseOverWidget)
    {
      Cog* mouseOverCog = mouseOverWidget->GetOwner();
      String clickEventId = NamedMouseClick[button];
      mouseOverCog->DispatchEvent(clickEventId, e);
      mouseOverCog->DispatchUp(clickEventId, e);

      // Send the DoubleClick event if we're within the time limit
      bool buttonsAreTheSame = (mLastClickedButton == button);
      bool distanceIsSmall = GetLargestAxis(e->Position - mLastClickPosition) < 4.0f;
      bool doubleClickTime = mTimeSinceLastClick < mDoubleClickTime;

      if(buttonsAreTheSame && distanceIsSmall && doubleClickTime)
      {
        mouseOverCog->DispatchEvent(Events::DoubleClick, e);
        mouseOverCog->DispatchUp(Events::DoubleClick, e);
      }

      // Update state for double clicks
      mLastClickedButton = button;
      mTimeSinceLastClick = 0;
      mLastClickPosition = e->Position;
    }
  }

  // Send the event to the mouse over object
  if(mouseOverWidget)
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

//******************************************************************************
void UiRootWidget::MouseMove(ViewportMouseEvent* e)
{
  // Bring the reactive event into root space
  Vec3 worldPos = e->mHitPosition;
  Vec2 rootPos = mWidget->WorldToRoot(worldPos);

  // We want to send mouse events to the widget that the mouse is over
  UiWidget* newMouseOver = mWidget->CastPoint(rootPos);
  while(newMouseOver && !newMouseOver->GetInteractive())
    newMouseOver = newMouseOver->GetParentWidget();
  
  MouseOver(e, newMouseOver);
}

//******************************************************************************
void UiRootWidget::MouseOver(ViewportMouseEvent* e, UiWidget* newMouseOver)
{
  UiWidget* oldMouseOver = mMouseOverWidget;

  // If the mouse has moved to a new object, we want to send MouseEnter and Exit
  // events to both object
  if(newMouseOver != oldMouseOver)
  {
    Cog* oldMouseOverCog = (oldMouseOver) ? oldMouseOver->GetOwner() : nullptr;
    Cog* newMouseOverCog = (newMouseOver) ? newMouseOver->GetOwner() : nullptr;

    //ViewportMouseEvent mouseOut(*e);
    //mouseOut.Source = oldMouseOverCog
    //ViewportMouseEvent mouseIn(*e);
    //mouseOut.Source = newMouseOverCog

    cstr op = (mDebugMouseInteraction) ? "Over" : nullptr;

    if(newMouseOver)
      e->mHitObject = newMouseOver->GetOwner();

    SendHierarchyEvents(op, oldMouseOverCog, newMouseOverCog, e, e,
      Events::MouseExit, Events::MouseEnter,
      Events::MouseExitHierarchy, Events::MouseEnterHierarchy,
      UiWidgetFlags::MouseOver, UiWidgetFlags::MouseOverHierarchy,
      &WidgetFlagCallback);

    mMouseOverWidget = newMouseOver;
    //DebugPrint("Mouse now over %s\n", newMouseOverCog->mObjectName.c_str());
    mCurrHoverTime = 0.0f;
    mCurrHoldTime = 0.0f;
  }
}

//******************************************************************************
void UiRootWidget::RootChangeFocus(UiWidget* newFocus)
{
  UiWidget* oldFocus = mFocusWidget;

  //Do not change it the object is already the focus object
  if(oldFocus != newFocus)
  {
    Cog* oldFocusCog = (oldFocus) ? oldFocus->GetOwner() : nullptr;
    Cog* newFocusCog = (newFocus) ? newFocus->GetOwner() : nullptr;

    cstr op = (mDebugMouseInteraction) ? "Focus" : nullptr;

    // Send the Focus to the Hierarchy
    UiFocusEvent focusEvent(newFocusCog, oldFocusCog);

    SendHierarchyEvents(op, oldFocusCog, newFocusCog, &focusEvent, &focusEvent,
                    Events::UiFocusLost, Events::UiFocusGained, 
                    Events::UiFocusLostHierarchy, Events::UiFocusGainedHierarchy,
                    UiWidgetFlags::HasFocus, UiWidgetFlags::HierarchyHasFocus,
                    &WidgetFlagCallback);

    // Store the current focus object
    mFocusWidget = newFocus;
  }
}

//******************************************************************************
void UiRootWidget::DispatchAt(DispatchAtParams& params)
{
  //Widget* hit = mWidget->CastPoint(params.Position, params.Ignore);
  //
  //if(hit)
  //{
  //  if(params.BubbleEvent)
  //    hit->DispatchBubble(params.EventId, params.EventObject);
  //  else
  //    hit->DispatchEvent(params.EventId, params.EventObject);
  //
  //  params.ObjectHit = true;
  //}
}

//******************************************************************************
void UiRootWidget::SetDebugSelected(Cog* selected)
{
  mDebugSelectedWidget = selected;
}

//******************************************************************************
Cog* UiRootWidget::GetDebugSelected()
{
  return mDebugSelectedWidget.GetOwner();
}

}//namespace Zero
