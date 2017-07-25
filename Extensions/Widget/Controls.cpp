///////////////////////////////////////////////////////////////////////////////
///
/// \file Controls.cpp
/// Implementation of the basic Widget system controls.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(ValueChanged);
}//namespace Events

const String cButtonOver = "BackgroundOver";
const String cButtonDown = "BackgroundDown";

void TabJump(Widget* widget, KeyboardEvent* event)
{
  if(event->Key == Keys::Tab)
  {
    if(event->ShiftPressed)
      FindNextFocus(widget, FocusDirection::Backwards);
    else
      FindNextFocus(widget, FocusDirection::Forward);

    event->Handled = true;
  }
}

//----------------------------------------------------------------------- Spacer
ZilchDefineType(Spacer, builder, type)
{
}

const String SpacerName = "Spacer";

Spacer::Spacer(Composite* parent)
  : Widget(parent)
{
  mMinSize = Vec2(1,1);
  SetSizing(SizePolicy::Flex, 1);
  SetName("Spacer");
}

Spacer::Spacer(Composite* parent, SizePolicy::Enum policy, Vec2Param size)
  : Widget(parent)
{
  mMinSize = Vec2(1,1);
  SetSizing(policy, size);
  SetName("Spacer");
}

Spacer::~Spacer()
{

}

Vec2 Spacer::GetMinSize()
{
  return mMinSize;
}

void Spacer::SetSize(Vec2 size)
{
  mSize = size;
  mMinSize = size;
}

//------------------------------------------------------------------------- Splitter
ZilchDefineType(Splitter, builder, type)
{
}

const String SplitterClass = "Sash";

Splitter::Splitter(Composite* parent)
  : Composite(parent)
{
  mDragging = false;
  mReverse = false;
  mAxis = SizeAxis::X;

  mDefSet = mDefSet->GetDefinitionSet(SplitterClass);
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetInteractive(true);
  mBackground->SetColor(Vec4(0.139216, 0.139216, 0.139216, 1));

  ConnectThisTo(mBackground, Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(mBackground, Events::LeftMouseUp, OnLeftMouseUp);
  ConnectThisTo(mBackground, Events::MouseMove, OnMouseMove);
  ConnectThisTo(mBackground, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(mBackground, Events::MouseExit, OnMouseExit);
}

Splitter::~Splitter()
{

}

void Splitter::UpdateTransform()
{
  Vec2 lineSize = GetSize() - Vec2(2,2);
  mBackground->SetSize(lineSize);
  mBackground->SetTranslation(Pixels(1,1,0));
  Composite::UpdateTransform();
}

Vec2 Splitter::GetMinSize()
{
  return Vec2(3,3);
}

void Splitter::OnLeftMouseDown(MouseEvent* event)
{
  mDragging = true;
  mMouseStart = event->GetMouse()->GetClientPosition();
  mBackground->CaptureMouse();

  Widget* left = mWidgetLink.Prev;
  Widget* right = mWidgetLink.Next;

  //Save the initial size of the left and right widget.
  //Used so we can properly deal with clamping sizes.
  mLeftStartSize = left->GetSize();
  mRightStartSize = right->GetSize();
  mLeftStartMinSize = Vec2(10,10); // left->GetMinSize();
  mRightStartMinSize = Vec2(10,10); // right->GetMinSize();
}

void Splitter::OnLeftMouseUp(MouseEvent* event)
{
  ReleaseMouseCapture();
  mDragging = false;
}

void SetSizeFlex(Widget* widget, SizeAxis::Enum axis, Vec2 newSize)
{
 // ErrorIf(widget->mSizePolicy.Policy[axis] == SizePolicy::Auto, "Can not size Auto");
  widget->mSizePolicy.Size[axis] = newSize[axis];
  widget->mSize[axis] = newSize[axis];
}

void Splitter::OnMouseMove(MouseEvent* event)
{
  if(!mDragging)
    return;

  Widget* left = mWidgetLink.Prev;
  Widget* right = mWidgetLink.Next;

  Widget* end = GetParent()->mChildren.End();
  Vec2 mousePos = event->GetMouse()->GetClientPosition();
  
  //compute how much the mouse has moved since the first down click
  Vec2 clampedMovement = mousePos - mMouseStart;
  //if we're in reverse mode, just flip the sign
  if(mReverse)
    clampedMovement *= -1;

  //for the current resizing axis, check and see if we're shrinking the left or right
  if(clampedMovement[mAxis] < 0 && left != end)
  {
    //we're shrinking the left, so compute the new size
    float newSize = mLeftStartSize[mAxis] + clampedMovement[mAxis];
    //however we can't go too small, so clamp to the max size
    newSize = Math::Max(newSize, mLeftStartMinSize[mAxis]);
    //now compute how much we actually moved
    clampedMovement[mAxis] = newSize - mLeftStartSize[mAxis];
  }
  else if(right != end)
  {
    //same as the left, except we have to have a few extra minus signs
    float newSize = mRightStartSize[mAxis] - clampedMovement[mAxis];
    newSize = Math::Max(newSize, mRightStartMinSize[mAxis]);
    clampedMovement[mAxis] = -(newSize - mRightStartSize[mAxis]);
  }

  //apply the clamped deltas to get the new size
  if(left != end)
    SetSizeFlex(left, mAxis, mLeftStartSize + clampedMovement);
  if(right != end)
    SetSizeFlex(right, mAxis, mRightStartSize - clampedMovement);

  //have to mark our parent as needing update, otherwise the animation will not be smooth
  this->GetParent()->MarkAsNeedsUpdate();
}

void Splitter::OnMouseEnter(MouseEvent* event)
{
  if(mAxis == SizeAxis::X)
    event->GetMouse()->SetCursor(Cursor::SizeWE);
  else
    event->GetMouse()->SetCursor(Cursor::SizeNS);
}

void Splitter::OnMouseExit(MouseEvent* event)
{
  event->GetMouse()->SetCursor(Cursor::Arrow);
}

}//namespace Zero
