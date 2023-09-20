// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
DefineEvent(MouseDragStart);
DefineEvent(MouseDragMove);
DefineEvent(MouseDragUpdate);
DefineEvent(MouseDragEnd);
} // namespace Events

class MouseCaptureDrag : public MouseManipulation
{
public:
  CogId mMouseCaptureObject;
  HandleOf<ReactiveViewport> mViewport;

  MouseCaptureDrag(ViewportMouseEvent* e, Cog* captureObject) : MouseManipulation(e->GetMouse(), e->GetViewport())
  {
    mViewport = e->GetViewport();
    mMouseCaptureObject = captureObject;

    // Button could not be determined in 'MouseManipulation' constructor,
    // because a mouse up event could have triggered this drag.
    // [ex: Toggle to drag].
    if (mButton == MouseButtons::None)
      mButton = e->Button;
  }

  ~MouseCaptureDrag()
  {
  }

  void ForwardMouseEvent(MouseEvent* e)
  {
    ForwardMouseEvent(e, e->EventId);
  }

  void ForwardMouseEvent(MouseEvent* e, StringParam eventId)
  {
    ReactiveViewport* viewport = *mViewport;
    if (viewport == NULL)
      return;

    // Create the viewport event
    ViewportMouseEvent eventToSend(e);
    eventToSend.EventId = eventId;
    viewport->InitViewportEvent(eventToSend);

    // Send the event
    ForwardEvent(&eventToSend);
    e->Handled = true;
    e->HandledEventScript = eventToSend.HandledEventScript;
  }

  void ForwardEvent(Event* e)
  {
    Cog* captureObject = mMouseCaptureObject;
    if (captureObject == nullptr)
      return;

    Debug::ActiveDrawSpace debugContext(captureObject->GetSpace()->GetId().Id);

    // Send the event on the object
    captureObject->DispatchEvent(e->EventId, e);
  }

  void OnMouseButtonUp(MouseEvent* e)
  {
    Cog* captureObject = mMouseCaptureObject;
    if (captureObject == nullptr)
      return;

    if (MouseCapture* capture = captureObject->has(MouseCapture))
    {
      // The button that created the capture is the one that has to end it.
      if (mButton == e->Button)
      {
        ReactiveViewport* viewport = *mViewport;

        // Create the viewport event
        ViewportMouseEvent eventToSend(e);
        eventToSend.EventId = e->EventId;

        if (viewport != nullptr)
          viewport->InitViewportEvent(eventToSend);

        capture->ReleaseCapture(&eventToSend);

        e->Handled = eventToSend.Handled;
        e->HandledEventScript = eventToSend.HandledEventScript;
      }
    }
  }

  void OnMouseUp(MouseEvent* e) override
  {
    OnMouseButtonUp(e);
  }

  void OnMouseDown(MouseEvent* e) override
  {
    ForwardMouseEvent(e);
  }

  void OnMouseMove(MouseEvent* e) override
  {
    ForwardMouseEvent(e, Events::MouseDragMove);
  }

  void OnRightMouseUp(MouseEvent* e) override
  {
    OnMouseButtonUp(e);
  }

  void OnMiddleMouseUp(MouseEvent* e) override
  {
    OnMouseButtonUp(e);
  }

  void OnMouseUpdate(MouseEvent* e) override
  {
    ForwardMouseEvent(e, Events::MouseDragUpdate);
  }

  void OnKeyDown(KeyboardEvent* e) override
  {
    Viewport* viewport = mViewport;
    if (viewport == NULL)
      return;

    // viewport->DispatchEvent(Events::KeyDown, e);
    ForwardEvent(e);

    // Event should be considered handled if the mouse is captured.
    //   - Ex: A "Ctrl+Z" keyboard event left unhandled could cause an
    //         OperationQueue::Undo to fire off.  Yet, if a gizmo is currently
    //         being dragged then the undo is incorrect, unwanted behavior.
    e->Handled = true;
  }

  void OnKeyUp(KeyboardEvent* e) override
  {
    Viewport* viewport = mViewport;
    if (viewport == NULL)
      return;

    // viewport->DispatchEvent(Events::KeyUp, e);
    ForwardEvent(e);
    e->Handled = true;
  }

  void OnKeyRepeated(KeyboardEvent* e) override
  {
    Viewport* viewport = mViewport;
    if (viewport == NULL)
      return;

    // viewport->DispatchEvent(Events::KeyRepeated, e);
    ForwardEvent(e);
    e->Handled = true;
  }
};

RaverieDefineType(MouseCapture, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();

  RaverieBindMethod(Capture);
  RaverieBindOverloadedMethod(ReleaseCapture, RaverieInstanceOverload(void));
  RaverieBindOverloadedMethod(ReleaseCapture, RaverieInstanceOverload(void, ViewportMouseEvent*));
  RaverieBindGetter(IsCaptured);

  RaverieBindEvent(Events::MouseDragStart, ViewportMouseEvent);
  RaverieBindEvent(Events::MouseDragMove, ViewportMouseEvent);
  RaverieBindEvent(Events::MouseDragUpdate, ViewportMouseEvent);
  RaverieBindEvent(Events::MouseDragEnd, ViewportMouseEvent);
}

void MouseCapture::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner(), Events::MouseDragUpdate, OnMouseDragUpdate);
}

bool MouseCapture::Capture(ViewportMouseEvent* e)
{
  if (GetIsCaptured())
    return false;

  // Create the viewport event
  ViewportMouseEvent eventToSend(e);
  ReactiveViewport* viewport = e->GetViewport();
  viewport->InitViewportEvent(eventToSend);

  GetOwner()->DispatchEvent(Events::MouseDragStart, &eventToSend);
  GetOwner()->DispatchUp(Events::MouseDragStart, &eventToSend);

  mManipulation = new MouseCaptureDrag(e, GetOwner());
  e->Handled = true;
  return true;
}

void MouseCapture::ReleaseCapture()
{
  ReleaseCapture(&mLastMouseEvent);
}

void MouseCapture::ReleaseCapture(ViewportMouseEvent* e)
{
  // Don't do anything if we weren't captured
  if (GetIsCaptured() == false)
    return;

  GetOwner()->DispatchEvent(Events::MouseDragEnd, e);
  GetOwner()->DispatchUp(Events::MouseDragEnd, e);

  mManipulation.SafeDestroy();
}

bool MouseCapture::GetIsCaptured()
{
  return mManipulation.IsNotNull();
}

void MouseCapture::OnDestroy(u32 flags)
{
  mManipulation.SafeDestroy();
}

void MouseCapture::OnMouseDragUpdate(ViewportMouseEvent* e)
{
  mLastMouseEvent = *e;
}

} // namespace Raverie
