///////////////////////////////////////////////////////////////////////////////
///
/// \file Manipulator.cpp
/// Implementation of the MouseManipulation and Gripper.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(MouseManipulation, builder, type)
{
}

//----------------------------------------------------------- Mouse Manipulation
const String MouseManipulationElement = "MouseManipulation";

MouseManipulation::MouseManipulation(Mouse* mouse, Composite* parent)
  : Widget(parent, AttachType::Direct) // Manipulation should not be attached to the client area
{
  SetNotInLayout(true);

  mMouse = mouse;
  mRelative = parent;

  this->CaptureMouse();

  mMouseStartPosition = mouse->GetClientPosition();

  // If a button triggered this MouseManipulation, determine which button.
  //   - ie, a mouse button click isn't the only way to start a MouseManipulation
  //   - ex: MouseMove
  for(uint i = MouseButtons::Left; i < MouseButtons::Size; ++i)
  {
    MouseButtons::Enum button = MouseButtons::Enum(i);

    // Only ONE button, in order of precedence, may claim this MouseManipulation.
    if(mouse->IsButtonDown(button))
    {
      mButton = button;

      ErrorIf(mButton == MouseButtons::None,
        "A valid MouseButton MUST trigger an instance of 'MouseManipulation'");

      break;
    }
    else  // No way to check if a MouseUp triggered this manipulation...
    {
      // Could cause bugs to always assume a MouseDown triggers the manipulation. So,
      // each class deriving from 'MouseManipulation' should handle those special cases.
      mButton = MouseButtons::None;
    }

  }

  ConnectThisTo(this, Events::FocusReset, OnFocusReset);
  ConnectThisTo(this, Events::FocusLost, OnFocusReset);

  ConnectThisTo(this, Events::MouseMove, OnMouseMove);

  ConnectThisTo(this, Events::LeftMouseUp, OnMouseUp);
  ConnectThisTo(this, Events::RightMouseUp, OnRightMouseUp);
  ConnectThisTo(this, Events::MiddleMouseUp, OnMiddleMouseUp);

  ConnectThisTo(this, Events::LeftMouseDown, OnMouseDown);

  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
  ConnectThisTo(this, Events::KeyUp, OnKeyUp);
  ConnectThisTo(this, Events::MouseScroll, OnMouseScroll);

  ConnectThisTo(this, Events::MouseUpdate, OnMouseUpdate);
  ConnectThisTo(GetRootWidget(), Events::WidgetUpdate, OnUpdate);
}

void MouseManipulation::ReleaseCapture()
{
  ReleaseMouseCapture();
  mMouse->SetCursor(Cursor::Arrow);
}

void MouseManipulation::CloseAndReturnFocus()
{
  Widget* relativeWidget = mRelative;
  mRelative = nullptr;
  if(relativeWidget)
    relativeWidget->TryTakeFocus();
  this->Destroy();
}

void MouseManipulation::OnFocusReset(FocusEvent* event)
{
  CloseAndReturnFocus();
}

void MouseManipulation::OnMouseUp(MouseEvent* event)
{
  CloseAndReturnFocus();
}

MouseManipulation::~MouseManipulation()
{
  ReleaseCapture();
}

//---------------------------------------------------------------------- Gripper

const String GripperElement = "Gripper";

Gripper::Gripper(Composite* parent, Widget* target, DockMode::Enum gripDirection)
  : Widget(parent, AttachType::Direct)
{
  mDefSet = parent->GetDefinitionSet();

  mTarget = target;
  mMouseOver = false;
  mGripDirection = (DockMode::Enum)gripDirection;
  mCurDockMode = (DockMode::Enum)gripDirection;

  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);
  ConnectThisTo(this, Events::LeftMouseDown, OnMouseDown);
  ConnectThisTo(this, Events::LeftMouseDrag, OnMouseDrag);
}

void Gripper::OnMouseDrag(MouseEvent* event)
{

}

Gripper::~Gripper()
{

}

Cursor::Enum DockDirectionToMouseCursor(DockMode::Enum dockDirection)
{
  switch (dockDirection)
  {
  case DockMode::DockTop:
  case DockMode::DockBottom:
    return Cursor::SizeNS;
  case DockMode::DockLeft:
  case DockMode::DockRight:
    return Cursor::SizeWE;
  case DockMode::DockTop | DockMode::DockLeft:
  case DockMode::DockBottom | DockMode::DockRight:
    return Cursor::SizeNWSE;
  case DockMode::DockBottom | DockMode::DockLeft:
  case DockMode::DockTop | DockMode::DockRight:
    return Cursor::SizeNESW;
  default:
    return Cursor::Arrow;
  }
}

Cursor::Enum Gripper::GetMouseCursor()
{
  return DockDirectionToMouseCursor(mGripDirection);
}

void Gripper::OnMouseEnter(MouseEvent* event)
{
  event->GetMouse()->SetCursor(GetMouseCursor());
}

void Gripper::OnMouseExit(MouseEvent* event)
{
  event->GetMouse()->SetCursor(Cursor::Arrow);
}

void Gripper::OnMouseDown(MouseEvent* event)
{
  Widget* target = mTarget;
  if(Docker* docker = target->GetDocker())
    docker->StartManipulation(target, mGripDirection);
  else
    new SizingManipulation(event->GetMouse(), target, mGripDirection);
}

GripZones::GripZones(Composite* owner, Widget * sizeTarget)
  :Composite(owner, AttachType::Direct)
{
  SetLayout(CreateEdgeDockLayout());

  mGripper[SlicesIndex::Top] = new Gripper(this, sizeTarget, DockMode::DockTop);
  mGripper[SlicesIndex::Left] = new Gripper(this, sizeTarget, DockMode::DockLeft);
  mGripper[SlicesIndex::Bottom] = new Gripper(this, sizeTarget,DockMode::DockBottom );
  mGripper[SlicesIndex::Right] = new Gripper(this, sizeTarget, DockMode::DockRight);

  mCornerGripper[0] = new Gripper(this, sizeTarget,
    DockMode::Enum(DockMode::DockTop | DockMode::DockLeft));
  mCornerGripper[1] = new Gripper(this, sizeTarget,
    DockMode::Enum(DockMode::DockTop | DockMode::DockRight));
  mCornerGripper[2] = new Gripper(this, sizeTarget,
    DockMode::Enum(DockMode::DockBottom | DockMode::DockLeft));
  mCornerGripper[3] = new Gripper(this, sizeTarget,
    DockMode::Enum(DockMode::DockBottom | DockMode::DockRight));
}

void GripZones::UpdateTransform() 
{
  Vec4 margins = Vec4(6, 6, 6, 6);

  Vec2 sideSize = Vec2(margins[SlicesIndex::Left], mSize.y);
  Vec2 topSize = Vec2(mSize.x, margins[SlicesIndex::Bottom]);
  Vec2 corSize = Vec2(margins[SlicesIndex::Left], margins[SlicesIndex::Bottom]);

  //Set up gripper areas
  mGripper[SlicesIndex::Top]->SetSize(topSize);
  mGripper[SlicesIndex::Left]->SetSize(sideSize);
  mGripper[SlicesIndex::Bottom]->SetSize(topSize);
  mGripper[SlicesIndex::Right]->SetSize(sideSize);

  for(uint i = 0; i < 4; ++i)
    mCornerGripper[i]->SetSize(corSize);

  Composite::UpdateTransform();
}


void MoveUpperAxis(int a, Vec3& newObjectPos, Vec2& newObjectSize, 
                   Vec2Param minSize, Vec2Param movement)
{
  //The max change is limited by the minimum size to
  //prevent shrinking the object to small
  float maxDelta = newObjectSize[a] - minSize[a];
  //The min change is limited by the position of the object
  //to prevent sizing outside the window
  float minDelta = -newObjectPos[a];

  //Clamp the value
  float moveAxis = Math::Clamp(movement[a], minDelta, maxDelta);

  //Update the position and size
  newObjectPos[a] += moveAxis;
  newObjectSize[a] -= moveAxis;
}

void MoveLowerAxis(int a, Vec3& newObjectPos, Vec2& newObjectSize, 
                   Vec2Param minSize, Vec2Param maxSize, Vec2Param movement)
{
  newObjectSize[a] += movement[a];
  newObjectSize[a] = Math::Clamp(newObjectSize[a], minSize[a], maxSize[a]);
}

//---------------------------------------------------------- Sizing Manipulation
SizingManipulation::SizingManipulation(Mouse* mouse, Widget* toBeSized, 
                                       DockMode::Enum mode)
  : MouseManipulation(mouse, toBeSized->GetParent())
{
  mBeingSized = toBeSized;
  mTargetStartPosition = toBeSized->GetTranslation();
  mTargetStartSize = toBeSized->GetSize();
  mSizerMode = mode;
}

void SizingManipulation::OnMouseUp(MouseEvent* event)
{
  event->GetMouse()->SetCursor(Cursor::Arrow);
  this->Destroy();
}

void SizingManipulation::OnMouseMove(MouseEvent* event)
{
  Widget* beingSized = mBeingSized;
  if(beingSized == nullptr)
  {
    // Lost widget while sizing
    this->Destroy();
    return;
  }

  Composite* parent = beingSized->GetParent();
  Vec2 newPos = event->Position;

  Vec2 delta = newPos - mMouseStartPosition;
  Vec3 newObjectPos = mTargetStartPosition;
  Vec2 newObjectSize = mTargetStartSize;
  Vec2 minSize = beingSized->GetMinSize();

  Vec2 parentSize = parent->GetSize();

  if(mSizerMode & DockMode::DockLeft)
    MoveUpperAxis(0, newObjectPos, newObjectSize, minSize, delta);

  if(mSizerMode & DockMode::DockTop)
    MoveUpperAxis(1, newObjectPos, newObjectSize, minSize, delta);

  Vec2 maxSize = parentSize - Vec2(newObjectPos.x, newObjectPos.y);

  if(mSizerMode & DockMode::DockRight)
    MoveLowerAxis(0, newObjectPos, newObjectSize, minSize, maxSize, delta);

  if(mSizerMode & DockMode::DockBottom)
    MoveLowerAxis(1, newObjectPos, newObjectSize, minSize, maxSize, delta);

  if(mSizerMode & DockMode::DockFill)
  {
    newObjectPos = newObjectPos + Vec3(delta.x, delta.y, 0);
    Vec2 maxPoint = parentSize - newObjectSize;
    Vec2 minPoint = Vec2(0, 0);
    newObjectPos.x = Math::Clamp(newObjectPos.x, minPoint.x, maxPoint.x);
    newObjectPos.y = Math::Clamp(newObjectPos.y, minPoint.y, maxPoint.y);
  }

  beingSized->SetTranslationAndSize(newObjectPos, newObjectSize);
  beingSized->UpdateTransformExternal();
}

}//namespace Zero
