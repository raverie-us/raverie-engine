///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------ Root Input Router
//******************************************************************************
ZilchDefineType(UiRootInputRouter, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(UiRootWidget);
}

//******************************************************************************
UiRootInputRouter::UiRootInputRouter()
{
  mIgnoreEvents = false;
}

//******************************************************************************
void UiRootInputRouter::Serialize(Serializer& stream)
{

}

//******************************************************************************
void UiRootInputRouter::Initialize(CogInitializer& initializer)
{
  mRootWidget = GetOwner()->has(UiRootWidget);

  // If we have a Reactive Component, listen for events that come through it
  if(GetOwner()->has(Reactive))
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

    // We want to the root widget to know when the mouse has exited the entire thing
    ConnectThisTo(GetOwner(), Events::MouseExit, OnMouseEvent);

    // Keyboard events
    ConnectThisTo(Keyboard::Instance, Events::KeyDown, OnKeyboardEvent);
    ConnectThisTo(Keyboard::Instance, Events::KeyRepeated, OnKeyboardEvent);
  }
}

//******************************************************************************
void UiRootInputRouter::OnMouseEvent(ViewportMouseEvent* e)
{
  if(mIgnoreEvents)
    return;

  mIgnoreEvents = true;
  mRootWidget->PerformMouseEvent(e);
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

//******************************************************************************
void UiRootInputRouter::OnMouseButton(ViewportMouseEvent* e)
{
  if(mIgnoreEvents)
    return;

  mIgnoreEvents = true;
  mRootWidget->PerformMouseButton(e);
  mIgnoreEvents = false;

  // See the comment above the Terminate call in 'OnMouseEvent'
  e->Terminate();
}

//******************************************************************************
void UiRootInputRouter::OnMouseUpdate(ViewportMouseEvent* e)
{
  if(mIgnoreEvents)
    return;

  mIgnoreEvents = true;
  mRootWidget->UpdateMouseTimers(0.016f, e);
  mIgnoreEvents = false;

  // See the comment above the Terminate call in 'OnMouseEvent'
  e->Terminate();
}

//******************************************************************************
void UiRootInputRouter::OnKeyboardEvent(KeyboardEvent* e)
{
  if(mIgnoreEvents)
    return;

  mIgnoreEvents = true;
  mRootWidget->PerformKeyboardEvent(e);
  mIgnoreEvents = false;

  // See the comment above the Terminate call in 'OnMouseEvent'
  //e->Terminate();
}

//******************************************************************************
void UiRootInputRouter::SetOsWindow(OsWindow* window)
{
  // Disconnect from all Reactive events
  DisconnectAll(GetOwner(), this);

  //ConnectThisTo(osWindow, Events::OsResized, OnOsResize);
  //ConnectThisTo(osWindow, Events::OsMouseDown, OnOsMouseDown);
  //ConnectThisTo(osWindow, Events::OsMouseUp, OnOsMouseUp);
  //ConnectThisTo(osWindow, Events::OsMouseMove, OnOsMouseMoved);
  //
  //ConnectThisTo(osWindow, Events::OsMouseScroll, OnOsMouseScroll);
  //
  //ConnectThisTo(osWindow, Events::OsKeyTyped, OnOsKeyTyped);
  //ConnectThisTo(osWindow, Events::OsKeyRepeated, OnOsKeyDown);
  //ConnectThisTo(osWindow, Events::OsKeyDown, OnOsKeyDown);
  //ConnectThisTo(osWindow, Events::OsKeyUp, OnOsKeyUp);
  //
  //ConnectThisTo(osWindow, Events::OsFocusGained, OnOsFocusGained);
  //ConnectThisTo(osWindow, Events::OsFocusLost, OnOsFocusLost);
  //
  //ConnectThisTo(osWindow, Events::OsMouseFileDrop, OnOsMouseDrop);
  //ConnectThisTo(osWindow, Events::OsPaint, OnOsPaint);
  //
  //ConnectThisTo(osWindow, Events::OsClose, OnClose);
}

}//namespace Zero
