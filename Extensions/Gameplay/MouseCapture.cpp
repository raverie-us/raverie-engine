///////////////////////////////////////////////////////////////////////////////
///
/// \file MouseCapture.cpp
/// Implementation of the MouseCapture Component.
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(MouseDragStart);
  DefineEvent(MouseDragMove);
  DefineEvent(MouseDragEnd);
}

//-------------------------------------------------------------- Tool Mouse Drag
class MouseCaptureDrag : public MouseManipulation
{
public:
  CogId mMouseCaptureObject;
  HandleOf<Viewport> mViewport;

  //****************************************************************************
  MouseCaptureDrag(ViewportMouseEvent* e, Cog* captureObject)
    : MouseManipulation(e->GetMouse(), e->GetViewport())
  {
    mViewport = e->GetViewport();
    mMouseCaptureObject = captureObject;

    // Button could not be determined in 'MouseManipulation' constructor,
    // because a mouse up event could have triggered this drag.
    // [ex: Toggle to drag].
    if(mButton == MouseButtons::None)
      mButton = e->Button;
  }

  //****************************************************************************
  ~MouseCaptureDrag()
  {
  }

  //****************************************************************************
  void ForwardMouseEvent(MouseEvent* e)
  {
    ForwardMouseEvent(e, e->EventId);
  }

  //****************************************************************************
  void ForwardMouseEvent(MouseEvent* e, StringParam eventId)
  {
    ReactiveViewport* viewport = Type::DynamicCast<ReactiveViewport*>((Viewport*)mViewport);
    if(viewport == NULL)
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

  //****************************************************************************
  void ForwardEvent(Event* e)
  {
    Cog* captureObject = mMouseCaptureObject;
    if(captureObject == NULL)
      return;

    Debug::ActiveDrawSpace debugContext(captureObject->GetSpace()->GetId().Id);

    // Send the event on the object
    captureObject->DispatchEvent(e->EventId, e);
  }

  //****************************************************************************
  void OnMouseUp(MouseEvent* e) override
  {
    Cog* captureObject = mMouseCaptureObject;
    if(captureObject == NULL)
      return;

    if(MouseCapture* capture = captureObject->has(MouseCapture))
    {
      // The button that created the capture is the one that has to end it.
      if(mButton == e->Button)
        capture->ReleaseCapture( );
    }

  }

  //****************************************************************************
  void OnMouseDown(MouseEvent* e) override
  {
    ForwardMouseEvent(e);
  }

  //****************************************************************************
  void OnMouseMove(MouseEvent* e) override
  {
    ForwardMouseEvent(e, Events::MouseDragMove);
  }

  //****************************************************************************
  void OnRightMouseUp(MouseEvent* e) override
  {
    Cog* captureObject = mMouseCaptureObject;
    if(captureObject == NULL)
      return;

    if(MouseCapture* capture = captureObject->has(MouseCapture))
    {
      // The button that created the capture is the one that has to end it.
      if(mButton == e->Button)
        capture->ReleaseCapture( );
    }

  }

  //****************************************************************************
  void OnMiddleMouseUp(MouseEvent* e) override
  {
    // Do not forward MiddleMouse if it didn't create this capture.
    if(mButton == e->Button)
      ForwardMouseEvent(e);
  }

  //****************************************************************************
  void OnMouseUpdate(MouseEvent* e) override
  {
    ForwardMouseEvent(e);
  }

  //****************************************************************************
  void OnKeyDown(KeyboardEvent* e) override
  {
    Viewport* viewport = mViewport;
    if(viewport == NULL)
      return;

    //viewport->DispatchEvent(Events::KeyDown, e);
    ForwardEvent(e);
  }

  //****************************************************************************
  void OnKeyUp(KeyboardEvent* e) override
  {
    Viewport* viewport = mViewport;
    if(viewport == NULL)
      return;

    //viewport->DispatchEvent(Events::KeyUp, e);
    ForwardEvent(e);
  }
};

//---------------------------------------------------------------- Mouse Capture

//******************************************************************************
ZilchDefineType(MouseCapture, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZilchBindMethod(Capture);
  ZilchBindMethod(ReleaseCapture);
  ZilchBindMethod(IsCaptured);

  ZeroBindEvent(Events::MouseDragStart, ViewportMouseEvent);
  ZeroBindEvent(Events::MouseDragMove, ViewportMouseEvent);
  ZeroBindEvent(Events::MouseDragEnd, Event);
}

//******************************************************************************
MouseCapture::~MouseCapture()
{

}

//******************************************************************************
void MouseCapture::Serialize(Serializer& stream)
{
}

//******************************************************************************
bool MouseCapture::Capture(ViewportMouseEvent* e)
{
  if(IsCaptured())
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

//******************************************************************************
void MouseCapture::ReleaseCapture()
{
  Event eventToSend;
  GetOwner()->DispatchEvent(Events::MouseDragEnd, &eventToSend);
  GetOwner()->DispatchUp(Events::MouseDragEnd, &eventToSend);

  mManipulation.SafeDestroy();
}

//******************************************************************************
bool MouseCapture::IsCaptured()
{
  return mManipulation.IsNotNull();
}

//******************************************************************************
void MouseCapture::OnDestroy(u32 flags)
{
  mManipulation.SafeDestroy();
}

} // namespace Zero
