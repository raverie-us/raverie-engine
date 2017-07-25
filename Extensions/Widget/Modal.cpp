///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//----------------------------------------------------------------------- Events
namespace Events
{
  DefineEvent(ModalClosed);
  DefineEvent(ModalConfirmResult);
  DefineEvent(ModalButtonPressed);
}

//------------------------------------------------------------------- Tweakables
namespace ModalUi
{
  const cstr cLocation = "EditorUi/Modal";
  Tweakable(Vec4,  BackgroundColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4,  StipColor,       Vec4(1,1,1,1), cLocation);
  Tweakable(float, StripHeight,     Pixels(64),    cLocation);
  Tweakable(Vec4,  TextColor,       Vec4(1,1,1,1), cLocation);
}

namespace ModalStripUi
{
  const cstr cLocation = "EditorUi/Modal/ModalStrip";
  Tweakable(Vec4,  StipColor,   Vec4(1,1,1,1), cLocation);
  Tweakable(float, StripHeight, Pixels(64),    cLocation);
}

ZilchDefineType(ModalConfirmEvent, builder, type)
{
}

ZilchDefineType(ModalButtonEvent, builder, type)
{
}

ZilchDefineType(Modal, builder, type)
{
}

//------------------------------------------------------------ ModalConfirmEvent
//******************************************************************************
ModalConfirmEvent::ModalConfirmEvent()
{
  mUserData = nullptr;
}

//------------------------------------------------------------------------ Modal
//******************************************************************************
Modal::Modal(Composite* parent, float fadeInTime) :
  Composite(parent, AttachType::Direct)
{
  mCloseOnBackgroundClicked = true;
  mCloseOnEscape = true;

  SetLayout(CreateStackLayout());

  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(ModalUi::BackgroundColor);

  // We don't want to be in our parents layout
  SetNotInLayout(true);

  // Fade in the modal
  SetColor(Vec4(1,1,1,0));
  ActionSequence* seq = new ActionSequence(this);
  seq->Add(Fade(this, Vec4(1), fadeInTime));

  ConnectThisTo(mBackground, Events::LeftClick, OnBackgroundClicked);
  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
}

//******************************************************************************
void Modal::UpdateTransform()
{
  // Always size ourself to our parent
  SetSize(GetParent()->GetSize());

  mBackground->SetSize(mSize);

  Composite::UpdateTransform();
}

//******************************************************************************
void Modal::SetBackgroundColor(Vec4Param color)
{
  mBackground->SetColor(color);
}

//******************************************************************************
void Modal::Close(float fadeOutTime)
{
  ActionSequence* seq = new ActionSequence(this);
  seq->Add(Fade(this, Vec4(1,1,1,0), fadeOutTime));
  seq->Add(new CallAction<Widget, &Widget::Destroy>(this));

  Event eventToSend;
  DispatchEvent(Events::ModalClosed, &eventToSend);
}

//******************************************************************************
void Modal::OnBackgroundClicked(Event*)
{
  if(mCloseOnBackgroundClicked)
    Close();
}

//******************************************************************************
void Modal::OnKeyDown(KeyboardEvent* e)
{
  if(e->Key == Keys::Escape && mCloseOnEscape)
    Close();
}

//------------------------------------------------------------------ Modal Strip
//******************************************************************************
ModalStrip::ModalStrip(Composite* parent, float fadeInTime) :
  Modal(parent, fadeInTime)
{
  mStrip = CreateAttached<Element>(cWhiteSquare);
  mStrip->SetColor(ModalUi::StipColor);

  // Spacer
  Composite* spacer = new Composite(this, AttachType::Direct);
  spacer->SetSizing(SizePolicy::Flex, Vec2(1));

  // Create the strip area in the center
  mStripArea = new Composite(this, AttachType::Direct);
  mStripArea->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
  SetStripHeight(ModalSizeMode::Fixed, ModalUi::StripHeight);

  // Spacer
  spacer = new Composite(this, AttachType::Direct);
  spacer->SetSizing(SizePolicy::Flex, Vec2(1));
}

//******************************************************************************
void ModalStrip::UpdateTransform()
{
  if(mStripHeightMode == ModalSizeMode::Percentage)
  {
    float currHeight = mSize.y * mStripHeight;
    mStripArea->SetSizing(SizeAxis::Y, SizePolicy::Fixed, currHeight);
  }

  mStrip->SetTranslation(mStripArea->mTranslation);
  mStrip->SetSize(mStripArea->mSize);

  Modal::UpdateTransform();
}

//******************************************************************************
void ModalStrip::AttachChildWidget(Widget* widget, AttachType::Enum attachType)
{
  if(attachType == AttachType::Direct)
    Modal::AttachChildWidget(widget);
  else
    mStripArea->AttachChildWidget(widget);
}

//******************************************************************************
void ModalStrip::SetStripColor(Vec4Param color)
{
  mStrip->SetColor(color);
}

//******************************************************************************
void ModalStrip::SetStripHeight(ModalSizeMode::Type mode, float height)
{
  mStripHeightMode = mode;
  mStripHeight = height;

  float currHeight = height;
  if(mode == ModalSizeMode::Percentage)
    currHeight = mSize.y * height;
  mStripArea->SetSizing(SizeAxis::Y, SizePolicy::Fixed, currHeight);

  MarkAsNeedsUpdate();
}

//--------------------------------------------------------- Modal Confirm Action
//******************************************************************************
ModalConfirmAction::ModalConfirmAction(Composite* parent, StringParam title,
                                       float fadeInTime) : 
  ModalStrip(parent, fadeInTime)
{
  mCloseOnSelection = true;

  mStripArea->SetLayout(CreateRowLayout());

  // Spacer
  new Spacer(mStripArea);

  Composite* center = new Composite(mStripArea);
  center->SetLayout(CreateStackLayout());
  {
    mTitle = new Text(center, "ModalConfirmTitle");
    mTitle->SetText(title);
    mTitle->SizeToContents();
    ProxyAndAnimateIn(mTitle, Pixels(-400, 0, 0), 0.22f, 0.1f, 0);

    Composite* buttons = new Composite(center);
    buttons->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(15, 0), Thickness::cZero));
    {
      // Spacer to right justify the buttons
      new Spacer(buttons);

      mConfirm = new TextButton(buttons, "ModalConfirmButton");
      mConfirm->SetText("CONFIRM");
      mConfirm->SetStyle(TextButtonStyle::Modern);
      mConfirm->SizeToContents();
      ProxyAndAnimateIn(mConfirm, Pixels(-600, 0, 0), 0.25f, 0.1f, 0.05f);
      ConnectThisTo(mConfirm, Events::ButtonPressed, OnConfirmPressed);

      mCancel = new TextButton(buttons, "ModalConfirmButton");
      mCancel->SetText("CANCEL");
      mCancel->SetStyle(TextButtonStyle::Modern);
      mCancel->SizeToContents();
      ProxyAndAnimateIn(mCancel, Pixels(-500, 0, 0), 0.25f, 0.1f, 0.025f);
      ConnectThisTo(mCancel, Events::ButtonPressed, OnCancelPressed);
    }
    buttons->SizeToContents();
  }

  // Spacer
  new Spacer(mStripArea);
}

//******************************************************************************
void ModalConfirmAction::Close(float fadeOutTime)
{
  SendEvent(false);
  ModalStrip::Close(fadeOutTime);
}

//******************************************************************************
void ModalConfirmAction::OnConfirmPressed(Event* e)
{
  Confirm();
}

//******************************************************************************
void ModalConfirmAction::OnCancelPressed(Event* e)
{
  Cancel();
}

//******************************************************************************
void ModalConfirmAction::Confirm()
{
  // Send the event
  SendEvent(true);

  if(mCloseOnSelection)
    ModalStrip::Close();
}

//******************************************************************************
void ModalConfirmAction::Cancel()
{
  SendEvent(false);
  if(mCloseOnSelection)
    ModalStrip::Close();
}

//******************************************************************************
void ModalConfirmAction::SendEvent(bool confirmed)
{
  // Send the confirm event on ourself
  ModalConfirmEvent eventToSend;
  eventToSend.mConfirmed = confirmed;
  eventToSend.mUserData = mUserData;
  eventToSend.mStringUserData = mStringUserData;
  GetDispatcher()->Dispatch(Events::ModalConfirmResult, &eventToSend);
}

//******************************************************************************
bool ModalConfirmAction::TakeFocusOverride()
{
  mConfirm->TakeFocus();
  return true;
}

//--------------------------------------------------------- Modal ModalButtonsAction Action
//******************************************************************************
ModalButtonsAction::ModalButtonsAction(Composite* parent, StringParam title, Array<String>& buttonNames, StringParam extraText, float fadeInTime) : ModalStrip(parent, fadeInTime)
{
  CreateButtons(title, buttonNames, extraText);
}

//******************************************************************************
ModalButtonsAction::ModalButtonsAction(Composite* parent, StringParam title, StringParam buttonName, StringParam extraText, float fadeInTime) : ModalStrip(parent, fadeInTime)
{
  Array<String> buttonNames;
  buttonNames.PushBack(buttonName);
  CreateButtons(title, buttonNames, extraText);
}

//******************************************************************************
void ModalButtonsAction::CreateButtons(StringParam title, Array<String>& buttonNames, StringParam extraText)
{
  SetStripHeight(ModalSizeMode::Fixed, ModalUi::StripHeight + Pixels(30));
  mCloseOnSelection = true;

  mStripArea->SetLayout(CreateRowLayout());

  // Spacer
  new Spacer(mStripArea);

  Composite* center = new Composite(mStripArea);
  center->SetLayout(CreateStackLayout());
  {
    mTitle = new Text(center, "ModalConfirmTitle");
    mTitle->SetText(title);
    mTitle->SizeToContents();
    ProxyAndAnimateIn(mTitle, Pixels(-400, 0, 0), 0.22f, 0.1f, 0);

    if(!extraText.Empty())
    {
      Composite* extraTextComposite = new Composite(center);
      extraTextComposite->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(15, 0), Thickness::cZero));
      Label* extraTextUi = new Label(extraTextComposite, "ModalConfirmExtraText");
      extraTextUi->SetName("ExtraText");
      extraTextUi->SetText(extraText);
      extraTextUi->SizeToContents();
      ProxyAndAnimateIn(extraTextUi, Pixels(-400, 0, 0), 0.22f, 0.1f, 0);
    }

    Composite* buttons = new Composite(center);
    buttons->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(15, 0), Thickness::cZero));

    // Spacer to right justify the buttons
    new Spacer(buttons);

    // Create all of the buttons
    for(size_t i = 0; i < buttonNames.Size(); ++i)
    {
      TextButton* button = new TextButton(buttons, "ModalConfirmButton");
      button->SetName(buttonNames[i]);
      button->SetText(buttonNames[i]);
      button->SetStyle(TextButtonStyle::Modern);
      button->SizeToContents();
      ProxyAndAnimateIn(button, Pixels(-600, 0, 0), 0.25f, 0.1f, 0.05f);

      mButtons.PushBack(button);
      ConnectThisTo(button, Events::ButtonPressed, OnButtonPressed);
    }
    buttons->SizeToContents();
  }

  // Spacer
  new Spacer(mStripArea);
}

//******************************************************************************
void ModalButtonsAction::Close(float fadeOutTime)
{
  ModalStrip::Close(fadeOutTime);
}

//******************************************************************************
void ModalButtonsAction::OnButtonPressed(ObjectEvent* e)
{
  TextButton* button = (TextButton*)e->Source;
  SendButtonPressed(button);

  if(mCloseOnSelection)
    Close();
}

//******************************************************************************
void ModalButtonsAction::SendButtonPressed(TextButton* button)
{
  // Send the confirm event on ourself
  ModalButtonEvent toSend;
  toSend.mButtonName = button->mName;
  toSend.mUserData = mUserData;
  toSend.mStringUserData = mStringUserData;
  GetDispatcher()->Dispatch(Events::ModalButtonPressed, &toSend);
}

//******************************************************************************
bool ModalButtonsAction::TakeFocusOverride()
{
  if(mButtons.Empty())
    mButtons[0]->TakeFocus();
  return true;
}

}//namespace Zero
