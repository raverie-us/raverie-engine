///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorViewport.cpp
/// Implementation of the EditorViewport class.
///
/// Authors: Chris Peters, Joshua Claeys, Nathan Carlson, Ryan Edgemon
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//----------------------------------------------------- Camera Editor Mouse Drag

EditorCameraMouseDrag::EditorCameraMouseDrag(Mouse* mouse, EditorViewport* editorViewport, EditorCameraController* controller)
  : MouseManipulation(mouse, editorViewport)
{
  mController = controller;
  mViewport = editorViewport->GetReactiveViewport( );
  mEditorViewport = editorViewport;

  ConnectThisTo(this, Events::RightMouseDown, OnMouseDown);
  ConnectThisTo(this, Events::MiddleMouseDown, OnMouseDown);
}

EditorCameraMouseDrag::EditorCameraMouseDrag(Mouse* mouse, Viewport* viewport, EditorCameraController* controller)
  : MouseManipulation(mouse, viewport)
{
  mController = controller;
  mViewport = viewport;

  ConnectThisTo(this, Events::RightMouseDown, OnMouseDown);
  ConnectThisTo(this, Events::MiddleMouseDown, OnMouseDown);
}

EditorCameraMouseDrag::~EditorCameraMouseDrag( )
{
  if(EditorCameraController* controller = mController)
    controller->EndDrag( );
}

void EditorCameraMouseDrag::OnMouseMove(MouseEvent* event)
{
  Viewport* viewport = mViewport;
  if(viewport == nullptr)
    return;

  Vec2 mouseDelta = event->Position - mMouseStartPosition;
  mMouseStartPosition = event->Position;

  EditorCameraController* controller = mController;
  if(controller)
  {
    controller->DragMovement(mouseDelta, viewport);

    if(controller->IsActive( ))
    {
      if(EditorViewport* viewport = mEditorViewport)
        viewport->mMouseOperation = MouseMode::Default;
    }

  }
  
  event->Handled = true;
}

void EditorCameraMouseDrag::OnMouseUp(MouseEvent* event)
{
  // Check that the event button is the same button that started the drag.
  if(event->Button == mButton)
    this->Destroy( );

  EditorCameraController* controller = mController;
  if(controller && controller->IsActive( ))
  {
    {
      if(EditorViewport* viewport = mEditorViewport)
        viewport->mMouseOperation = MouseMode::Default;
    }

  }

  event->Handled = true;
}

void EditorCameraMouseDrag::OnMouseDown(MouseEvent* event)
{
    // EditorCamera mouse drag/capture should have total authority and claim all events.
  event->Handled = true;
}

void EditorCameraMouseDrag::OnRightMouseUp(MouseEvent* event)
{
  OnMouseUp(event);
}

void EditorCameraMouseDrag::OnMiddleMouseUp(MouseEvent* event)
{
  OnMouseUp(event);
}

void EditorCameraMouseDrag::OnRightMouseDown(MouseEvent* event)
{
  OnMouseDown(event);
}

void EditorCameraMouseDrag::OnMiddleMouseDown(MouseEvent* event)
{
  OnMouseDown(event);
}

void EditorCameraMouseDrag::OnKeyDown(KeyboardEvent* event)
{
  if(EditorCameraController* controller = mController)
  {
    controller->ProcessKeyboardEvent(event);
    event->Handled = true;
  }

}

void EditorCameraMouseDrag::OnKeyUp(KeyboardEvent* event)
{
  if(EditorCameraController* controller = mController)
  {
    controller->ProcessKeyboardEvent(event);
    event->Handled = true;
  }

}

void EditorCameraMouseDrag::OnTargetDestroy(MouseEvent* event)
{
  this->Destroy( );
}


} // namespace Zero