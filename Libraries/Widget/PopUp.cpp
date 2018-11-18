///////////////////////////////////////////////////////////////////////////////
///
/// \file PopUp.cpp
/// Implementation of the PopUp.
///
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const String cPopUpNormal = "ItemPopUp";
const String cPopUpLight =  "ItemPopUpLight";

namespace PopUpUi
{
const cstr cLocation = "EditorUi/Controls/Popup";
Tweakable(Vec4, BackgroundColor, Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, BorderColor, Vec4(1,1,1,1), cLocation);
}

namespace Events
{
  DefineEvent(PopUpClosed);
}

ZilchDefineType(FloatingComposite, builder, type)
{
}

ZilchDefineType(PopUp, builder, type)
{
}

FloatingComposite::FloatingComposite(Composite* parent, StringParam className)
 : Composite(parent)
{
  mDefSet = mDefSet->GetDefinitionSet(className);
  mDropShadow = CreateAttached<Element>(cDropShadow);
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBorder = CreateAttached<Element>(cWhiteSquareBorder);
  mBorder->SetInteractive(false);
  mBorder->SetColor(PopUpUi::BorderColor);
  mDropShadow->SetInteractive(false);
}

void FloatingComposite::UpdateTransform()
{
  mBackground->SetSize(mSize);
  mBackground->SetColor(PopUpUi::BackgroundColor);
  mDropShadow->SetSize(mSize);
  mDropShadow->SetTranslation( Pixels(6,6,0) );
  mBorder->SetSize(mSize);
  Composite::UpdateTransform();
}

void FloatingComposite::FadeIn(float time)
{
  this->SetColor(Vec4(1,1,1,0));
  ActionSequence* seq = new ActionSequence(this);
  seq->Add( Fade(this, Vec4(1,1,1,1), time) );
  this->GetParent()->MarkAsNeedsUpdate();
  this->SetSize( Pixels(180,48));
}

void FloatingComposite::FadeOut(float time)
{
  ActionSequence* seq = new ActionSequence(this);
  seq->Add( Fade(this, Vec4(1,1,1,0), time) );
  seq->Add( DestroyAction(this) );

  ObjectEvent eventToSend(this);
  DispatchEvent(Events::PopUpClosed, &eventToSend);
}

void FloatingComposite::Slide(Vec3Param offset, float time)
{
  Vec3 destinatino = GetTranslation() + offset;
  AnimateTo(this, destinatino, GetSize(), time);
}

PopUp::PopUp(Widget* target, PopUpCloseMode::Enum popCloseMode, StringParam className)
 : FloatingComposite(target->GetRootWidget()->GetPopUp(), className )
{
  mMoved = false;
  mCloseMode = popCloseMode;
  mTarget = target;

  Keyboard* keyboard = Keyboard::GetInstance();

  ConnectThisTo(keyboard, Events::KeyDown, OnKeyDown);
  ConnectThisTo(this, Events::FocusLostHierarchy, OnFocusOut);
  ConnectThisTo(target, Events::MouseExitHierarchy, OnTargetMouseExit);

  Widget* root = target->GetRootWidget();

  ConnectThisTo(root, Events::FocusGained, OnAnyGained);
  ConnectThisTo(root, Events::MouseMove, OnMouseMove);
  ConnectThisTo(root, Events::MouseDown, OnMouseDown);
}

void PopUp::SetBelowMouse(Mouse* mouse, Vec2 offset)
{
  Vec3 screenPosition = GetScreenPosition();
  Vec2 screenSize = this->GetRootWidget()->GetSize();
  Vec2 thisSize = this->GetSize();

  Vec3 newPosition = ToVector3(mouse->GetClientPosition());
  newPosition+= Vec3(offset.x, offset.y, 0);

  //Move in from the right
  if(newPosition.x + thisSize.x > screenSize.x)
    newPosition.x -= thisSize.x;

  //Move from bottom
  if(newPosition.y + thisSize.y > screenSize.y)
    newPosition.y -= thisSize.y;

  this->SetTranslation(newPosition);
}

void PopUp::ShiftOntoScreen(Vec3 offset)
{
  Composite::ShiftOntoScreen(offset);
}

void PopUp::OnMouseDown(MouseEvent* event)
{
  if(!mMoved)
    return;

  // If the mouse down out side the pop up close it
  if(!this->Contains(event->Position))
    FadeOut();
}

void PopUp::OnAnyGained(FocusEvent* event)
{
  // Did focus move outside the popup?
  if(!this->IsAncestorOf(event->ReceivedFocus))
    FadeOut();
}

void PopUp::OnMouseMove(MouseEvent* event)
{
  mMoved = true;
  // Is the hover object still alive?
  Widget* hoverTarget = mTarget;
  if(hoverTarget == nullptr)
    FadeOut();

  // If the cursor move farther away from the center 
  // than the diagonal size close the pop up
  Vec3 localMousePos = mBackground->ToLocal(ToVector3(event->Position - mBackground->GetSize()*0.5f));
  bool notChildOfTarget = !hoverTarget->IsAncestorOf(event->Source);
  bool notChildOfMine = !this->IsAncestorOf(event->Source);
  if(notChildOfTarget && notChildOfMine && mCloseMode != PopUpCloseMode::DisableClose)
  {
    if(localMousePos.Length() > mSize.Length() && mCloseMode == PopUpCloseMode::MouseDistance)
    {
      FadeOut();
    }
  }
}

void PopUp::OnKeyDown(KeyboardEvent* event)
{
  //FadeOut if escape is pressed.
  if(event->Key == Keys::Escape)
    FadeOut();
}

void PopUp::OnTargetMouseExit(MouseEvent* event)
{
  if(mCloseMode == PopUpCloseMode::MouseOutTarget)
    FadeOut();
}

void PopUp::OnFocusOut(FocusEvent* event)
{
  FadeOut();
}

}
