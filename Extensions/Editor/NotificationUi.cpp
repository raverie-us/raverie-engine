///////////////////////////////////////////////////////////////////////////////
///
/// \file Notification.cpp
/// Implementation of the NotificationPopup class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace NotificationUi
{
const cstr cLocation = "EditorUi/Notification";
Tweakable(Vec4, BackgroundColor, Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, BorderColor, Vec4(1,1,1,1), cLocation);
}

static NotificationPopup* mNotifcationWindow = NULL;

//----------------------------------------------------------- Notification Popup

NotificationPopup::NotifyData::NotifyData(NotifyEvent* event)
{
  Type = event->Type;
  Name = event->Name;
  Message = event->Message;
  Icon = event->Icon;
}

NotificationPopup::NotificationPopup(Composite* composite, NotifyEvent* event)
  : Composite(composite)
{
  mMouseOver = false;
  mNotifyData.PushBack(NotifyData(event));

  static const String className = "TextButton";
  mDefSet = mDefSet->GetDefinitionSet(className);

  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBorder = CreateAttached<Element>(cWhiteSquareBorder);
  mText = new Text(this, cText);
  mText->SetMultiLine(true);
  mTitleText = new Label(this, cTitleText);
  
  mIcon = CreateAttached<Element>(event->Icon);
  if(mIcon==NULL)
    mIcon = CreateAttached<Element>("LargeEmpty");

  mNotInLayout = true;

  mText->SetText(event->Message);
  mTitleText->SetText(event->Name);

  mClose =  CreateAttached<Element>("Close");

  mBottomBar = new ColoredComposite(this, FloatColorRGBA(8, 8, 8, 255));
  mBottomBar->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness(1, 1, 12, 0)));
  {
    IconButton* leftButton = new IconButton(mBottomBar);
    leftButton->SetIcon("PreviousObject");
    leftButton->mBorder->SetVisible(false);
    leftButton->mBackgroundColor = ByteColorRGBA(0, 0, 0, 0);
    leftButton->SetSizing(SizePolicy::Fixed, Pixels(40, 16));
    ConnectThisTo(leftButton, Events::LeftClick, OnPreviousButton);

    IconButton* rightButton = new IconButton(mBottomBar);
    rightButton->SetIcon("NextObject");
    rightButton->mBorder->SetVisible(false);
    rightButton->mBackgroundColor = ByteColorRGBA(0, 0, 0, 0);
    rightButton->SetSizing(SizePolicy::Fixed, Pixels(40, 16));
    ConnectThisTo(rightButton, Events::LeftClick, OnNextButton);

    new Spacer(mBottomBar, SizePolicy::Flex, Vec2(1));

    mNumberOfNotificationsText = new Label(mBottomBar, cText);

  }
  mCurrentNotifyIndex = 0;

  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);

  ConnectThisTo(mClose, Events::LeftClick, OnCloseButton);

  composite->MarkAsNeedsUpdate();
}

NotificationPopup::~NotificationPopup()
{
  //there's only 1 notification window at a time,
  //so clear the static variable if we get destroyed
  mNotifcationWindow = NULL;
}

void NotificationPopup::SetNotifyEvent(NotifyEvent* event)
{
  //
}

void NotificationPopup::AddNotifyEvent(NotifyEvent* event)
{
  mNotifyData.PushBack(NotifyData(event));

  uint newIndex = mNotifyData.Size() - 1;
  NotifyType::Enum currErrType = mNotifyData[mCurrentNotifyIndex].Type;
  NotifyType::Enum newErrType = mNotifyData[newIndex].Type;

  //sort errors to take priority over warnings over general,
  //but if we are already on the same type don't change our index
  //also, if the mouse is not over the window, new items of the same priority will take over
  if(newErrType == NotifyType::Error)
  {
    if(currErrType != NotifyType::Error || mMouseOver == false)
    {
      mCurrentNotifyIndex = newIndex;
      ReadCurrentNotifyData();
    }
  }
  else if(newErrType == NotifyType::Warning)
  {
    if(currErrType == NotifyType::General || 
      (currErrType == NotifyType::Warning && mMouseOver == false))
    {
      mCurrentNotifyIndex = newIndex;
      ReadCurrentNotifyData();
    }
  }
  else if(currErrType == NotifyType::General && mMouseOver == false)
  {
    mCurrentNotifyIndex = newIndex;
    ReadCurrentNotifyData();
  }

  //a new notify happened, re-open the window
  AnimateToOpenThenClose();
}

void NotificationPopup::UpdateTransform()
{
  Vec2 sizeIcon = Vec2(0,0);
  mIcon->SetTranslation(Vec3(Pixels(10), Pixels(10), 0));
  sizeIcon = mIcon->GetSize();

  mClose->SetTranslation(Vec3(mSize.x - Pixels(16), Pixels(16), 0));

  float barHeight = Pixels(18);
  mBottomBar->SetTranslation(Vec3(0, mSize.y - barHeight, 0));
  mBottomBar->SetSize(Vec2(mSize.x, barHeight));

  mNumberOfNotificationsText->SetText(String::Format("(%d of %d)",mCurrentNotifyIndex + 1,mNotifyData.Size()));

  float xStart = sizeIcon.y + Pixels(10) + Pixels(10);
  mTitleText->SetTranslation(Vec3(xStart, Pixels(10), 0));
  mTitleText->SetSize(mSize);
  mText->SetTranslation(Vec3(xStart, Pixels(30), 0));
  mText->SetSize(Vec2(mSize.x - xStart - Pixels(20), mSize.y - Pixels(40)));

  mBackground->SetSize(mSize);
  mBackground->SetColor(NotificationUi::BackgroundColor);

  mBorder->SetSize(mSize);
  mBorder->SetColor(NotificationUi::BorderColor);

  Composite::UpdateTransform();
}

void NotificationPopup::ReadCurrentNotifyData()
{
  NotifyData& data = mNotifyData[mCurrentNotifyIndex];

  mText->SetText(data.Message);
  mTitleText->SetText(data.Name);
  mIcon->ChangeDefinition(mDefSet->GetDefinition(data.Icon));
  mIcon->SetSize(mIcon->GetMinSize());
}

Vec2 NotificationPopup::GetPopupSize()
{
  return Vec2(Pixels(300), Pixels(160));
}

void NotificationPopup::GetPositionAndSize(Vec2Ref pos, Vec2Ref size)
{
  Vec2 popUpSize = GetPopupSize();
  size = GetRootWidget()->GetSize();
  pos = size - popUpSize;
}

void NotificationPopup::GetStartAndEndPositions(Vec3Ref start, Vec3Ref end)
{
  Vec2 pos, size;
  GetPositionAndSize(pos,size);
  start = Vec3(pos.x, size.y, 0);
  end = Vec3(pos.x, pos.y, 0);
}

void NotificationPopup::AnimateToOpen()
{
  //clear out our old list then animate to an open position
  mActions->Cancel();

  Vec2 popUpSize = GetPopupSize();
  Vec3 start, end;
  GetStartAndEndPositions(start,end);

  ActionSequence* seq = new ActionSequence(this);
  seq->Add(MoveAndSizeWidgetAction(this, end, popUpSize, 0.3f));
}

void NotificationPopup::AnimateToClose()
{
  //add a delay, then animate to being closed
  Vec2 popUpSize = GetPopupSize();
  Vec3 start, end;
  GetStartAndEndPositions(start,end);

  ActionSequence* seq = new ActionSequence(this);
  seq->Add(new ActionDelay(5.0f));
  seq->Add(MoveAndSizeWidgetAction(this, start, popUpSize, 0.3f));
  seq->Add(DestroyAction(this));
}

void NotificationPopup::AnimateToOpenThenClose()
{
  //clear out our old list then animate to an open position
  if(mActions)
    mActions->Cancel();

  Vec2 popUpSize = GetPopupSize();
  Vec3 start, end;
  GetStartAndEndPositions(start,end);
  //
  ActionSequence* seq = new ActionSequence(this);
  seq->Add(MoveAndSizeWidgetAction(this, end, popUpSize, 0.3f));
  seq->Add(new ActionDelay(5.0f));
  seq->Add(MoveAndSizeWidgetAction(this, start, popUpSize, 0.3f));
  seq->Add(DestroyAction(this));
}

void NotificationPopup::OnMouseEnter(MouseEvent* event)
{
  mMouseOver = true;
  AnimateToOpen();
}

void NotificationPopup::OnMouseExit(MouseEvent* event)
{
  mMouseOver = false;
  AnimateToClose();
}

void NotificationPopup::OnCloseButton(MouseEvent* event)
{
  this->Destroy();
}

void NotificationPopup::OnPreviousButton(MouseEvent* event)
{
  if(mCurrentNotifyIndex != 0)
  {
    --mCurrentNotifyIndex;
    ReadCurrentNotifyData();
    MarkAsNeedsUpdate(true);
  }
}

void NotificationPopup::OnNextButton(MouseEvent* event)
{
  if(mCurrentNotifyIndex < mNotifyData.Size() - 1)
  {
    ++mCurrentNotifyIndex;
    ReadCurrentNotifyData();
    MarkAsNeedsUpdate(true);
  }
}

void DoNotifyPopup(Composite* root, NotifyEvent* event)
{
  //if we already have a window, add to it
  if(mNotifcationWindow != NULL)
  {
    mNotifcationWindow->AddNotifyEvent(event);
    return;
  }

  //otherwise create a new window and add to it that way
  mNotifcationWindow = new NotificationPopup(root, event);

  Vec2 popUpSize = mNotifcationWindow->GetPopupSize();
  Vec3 start, end;
  mNotifcationWindow->GetStartAndEndPositions(start,end);
  mNotifcationWindow->SetTranslationAndSize(start, popUpSize);

  mNotifcationWindow->AnimateToOpenThenClose();
}

}//namespace Zero
