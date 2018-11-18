///////////////////////////////////////////////////////////////////////////////
///
/// \file Notification.hpp
/// Declaration of the NotificationPopup class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//----------------------------------------------------------- Notification Popup
class NotificationPopup : public Composite
{
  struct NotifyData
  {
    NotifyData() {}
    NotifyData(NotifyEvent* event);

    NotifyType::Enum Type;
    String Name;
    String Message;
    String Icon;
  };

public:
  typedef NotificationPopup ZilchSelf;
  NotificationPopup(Composite* composite, NotifyEvent* event);
  ~NotificationPopup();

  void SetNotifyEvent(NotifyEvent* event);
  void AddNotifyEvent(NotifyEvent* event);

  void UpdateTransform();
  void ReadCurrentNotifyData();
  Vec2 GetPopupSize();
  void GetPositionAndSize(Vec2Ref pos, Vec2Ref size);
  void GetStartAndEndPositions(Vec3Ref start, Vec3Ref end);
  void AnimateToOpen();
  void AnimateToOpenThenClose();
  void AnimateToClose();

  void OnMouseEnter(MouseEvent* event);
  void OnMouseExit(MouseEvent* event);

  void OnCloseButton(MouseEvent* event);
  void OnPreviousButton(MouseEvent* event);
  void OnNextButton(MouseEvent* event);

private:
  Element* mBackground;
  Element* mBorder;
  Element* mIcon;
  Text* mText;
  Label* mTitleText;
  Element* mClose;
  Composite* mBottomBar;

  Array<NotifyData> mNotifyData;
  uint mCurrentNotifyIndex;

  Element* mPreviousButton;
  Element* mNextButton;

  Label* mNumberOfNotificationsText;
  bool mMouseOver;
};

void DoNotifyPopup(Composite* root, NotifyEvent* event);

}//namespace Zero
