///////////////////////////////////////////////////////////////////////////////
///
/// \file MetaDrop.cpp
/// 
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
  DefineEvent(MetaDrop);
  DefineEvent(MetaDropTest);
  DefineEvent(MetaDropUpdate);
}

//-------------------------------------------------------------- Meta Drop Event
ZilchDefineType(MetaDropEvent, builder, type)
{
  ZeroBindDocumented();

  ZeroBindEvent(Events::MetaDrop, MetaDropEvent);
  ZeroBindEvent(Events::MetaDropTest, MetaDropEvent);
  ZeroBindEvent(Events::MetaDropUpdate, MetaDropEvent);

  ZilchBindFieldProperty(Handled);
  ZilchBindFieldProperty(Failed);
  ZilchBindFieldProperty(Testing);
  ZilchBindFieldProperty(Position);
  ZilchBindFieldProperty(Result);
  ZilchBindGetterProperty(Object);
  ZilchBindGetterProperty(MouseOverObject);
  ZilchBindGetterProperty(MouseEvent);
  ZilchBindGetterProperty(ViewportMouseEvent);
}

//******************************************************************************
MetaDropEvent::MetaDropEvent(MouseEvent* e) :
  mMouseEvent(e), mViewportMouseEvent(nullptr)
{
  Property = nullptr;
  Testing = false;
  Handled = false;
  Failed = false;
  mUseTooltipPlacement = false;
}

//******************************************************************************
Handle MetaDropEvent::GetObject()
{
  return Instance;
}

//******************************************************************************
Handle MetaDropEvent::GetMouseOverObject()
{
  return MouseOverObject;
}

//******************************************************************************
MouseEvent* MetaDropEvent::GetMouseEvent()
{
  return mMouseEvent;
}

//******************************************************************************
ViewportMouseEvent* MetaDropEvent::GetViewportMouseEvent()
{
  return mViewportMouseEvent;
}

//-------------------------------------------------------------------- Meta Drag
//******************************************************************************
MetaDrag::MetaDrag(Mouse* mouse, Composite* owner, HandleParam object)
  : MouseManipulation(mouse, owner)
{
  AddObject(object);

  mToolTip = new ToolTip(owner->GetRootWidget());

  Keyboard* keyboard = Keyboard::GetInstance();
  ConnectThisTo(keyboard, Events::KeyDown, OnKeyDown);
}

//******************************************************************************
MetaDrag::~MetaDrag()
{
  mToolTip->Destroy();
}

//******************************************************************************
void MetaDrag::AddObject(HandleParam object)
{
  if(object.IsNotNull())
    mObjects.PushBack(object);
}

//******************************************************************************
void MetaDrag::OnMouseUp(MouseEvent* event)
{
  Vec2 position = event->GetMouse()->GetClientPosition();

  forRange(Handle object, mObjects.All())
  {
    MetaDropEvent* dropEvent = new MetaDropEvent(event);
    dropEvent->Position = event->Position;
    dropEvent->Instance = object;

    DispatchAtParams dispatchAtParams;
    dispatchAtParams.EventId = Events::MetaDrop;
    dispatchAtParams.EventObject = dropEvent;
    dispatchAtParams.Position = position;

    this->GetRootWidget()->DispatchAt(dispatchAtParams);

    delete dropEvent;
  }

  this->Destroy();
}

//******************************************************************************
void MetaDrag::OnKeyDown(KeyboardEvent* event)
{
  if(event->Key == Keys::Escape)
  {
    this->Destroy();
  }
}

//******************************************************************************
void MetaDrag::OnMouseMove(MouseEvent* event)
{
  Vec2 position = event->GetMouse()->GetClientPosition();

  forRange(Handle object, mObjects.All())
  {
    MetaDropEvent* dropEvent = new MetaDropEvent(event);
    dropEvent->Position = event->Position;
    dropEvent->Instance = object;
    dropEvent->Testing = true;

    DispatchAtParams dispatchAtParams;
    dispatchAtParams.EventId = Events::MetaDropTest;
    dispatchAtParams.EventObject = dropEvent;
    dispatchAtParams.Position = position;

    this->GetRootWidget()->DispatchAt(dispatchAtParams);

    if(dropEvent->Result.Empty())
    {
      mToolTip->SetActive(false);
      SafeDelete(dropEvent);
    }
    else
    {
      mToolTip->SetActive(true);
      mToolTip->SetText(dropEvent->Result);
      //If the drop event failed let the user know why
      if(dropEvent->Failed)
        mToolTip->SetColor(ToolTipColor::Red);
      else
        mToolTip->SetColor(ToolTipColor::Default);

      ToolTipPlacement placement;

      if(dropEvent->mUseTooltipPlacement)
      {
        placement = dropEvent->mToolTipPlacement;
      }
      else
      {
        placement.SetScreenRect(Rect::PointAndSize(position, Vec2::cZero));
        placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left, 
                              IndicatorSide::Bottom, IndicatorSide::Top);
      }
      
      mToolTip->SetArrowTipTranslation(placement);

      //Stop after first valid
      SafeDelete(dropEvent);
      return;
    }

  }
}

//******************************************************************************
void MetaDrag::OnMouseUpdate(MouseEvent* event)
{
  Vec2 position = event->GetMouse()->GetClientPosition();

  forRange(Handle object, mObjects.All())
  {
    MetaDropEvent* dropEvent = new MetaDropEvent(event);
    dropEvent->Position = event->Position;
    dropEvent->Instance = object;
    dropEvent->Testing = true;

    DispatchAtParams dispatchAtParams;
    dispatchAtParams.EventId = Events::MetaDropUpdate;
    dispatchAtParams.EventObject = dropEvent;
    dispatchAtParams.Position = position;

    this->GetRootWidget()->DispatchAt(dispatchAtParams);

    SafeDelete(dropEvent);
  }
}

}//namespace Zero
