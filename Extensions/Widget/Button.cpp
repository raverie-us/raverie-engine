///////////////////////////////////////////////////////////////////////////////
///
/// \file Button.cpp
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
  DefineEvent(ButtonPressed);
}

namespace ButtonUi
{
  const cstr cLocation = "EditorUi/Controls/Button";
  Tweakable(Vec4, DefaultColor,      Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, HoverColor,        Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, ClickedColor,      Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BorderColor,       Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, FocusBorderColor,  Vec4(1,1,1,1), cLocation);
}

namespace TextBUttonUi
{
  const cstr cLocation = "EditorUi/Controls/ModernTextButton";
  Tweakable(Vec4, TextColor,        Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, TextHoverColor,   Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, TextClickedColor, Vec4(1,1,1,1), cLocation);
}

namespace IconButtonUi
{
  const cstr cLocation = "EditorUi/Controls/IconButton";
  Tweakable(Vec4, DefaultColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, HoverColor,   Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, ClickedColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec2, Padding,      Vec2(2,2), cLocation);
  Tweakable(Vec4, ActiveColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, ActiveHoverColor, Vec4(1,1,1,1), cLocation);
}

//------------------------------------------------------------------ Button Base
ZilchDefineType(ButtonBase, builder, type)
{

}

ButtonBase::ButtonBase(Composite* parent, StringParam styleClass)
  : Composite(parent)
  , mToolTipColor(ToolTipColor::Default)
{
  mCommand = NULL;
  mDefSet = mDefSet->GetDefinitionSet(styleClass);

  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetTakeFocusMode(FocusMode::Hard);

  mBorder = CreateAttached<Element>(cWhiteSquareBorder);
  mFocusBorder = CreateAttached<Element>(cWhiteSquareBorder);
  mFocusBorder->SetVisible(false);
  mFocusBorder->SetInteractive(false);

  ConnectThisTo(this, Events::MouseEnterHierarchy, OnMouseEnter);
  ConnectThisTo(this, Events::MouseExitHierarchy, OnMouseExit);
  ConnectThisTo(this, Events::LeftMouseDown, OnMouseDown);
  ConnectThisTo(this, Events::LeftMouseUp, OnMouseUp);
  ConnectThisTo(this, Events::MouseHover, OnHover);
  ConnectThisTo(this, Events::LeftClick, OnMouseClick);
  ConnectThisTo(this, Events::KeyDown, OnKeyDown);

  ConnectThisTo(this, Events::FocusGained, OnFocusGained);
  ConnectThisTo(this, Events::FocusLost, OnFocusGained);

  mMouseDown = false;
  mMouseOver = false;
  mTabFocusStop = true;
  mIgnoreInput = false;
  mShowToolTip = true;

  mBorderColor = ToByteColor(ButtonUi::BorderColor);
  mFocusBorderColor = ToByteColor(ButtonUi::FocusBorderColor);
  mBackgroundColor = ToByteColor(ButtonUi::DefaultColor);
  mBackgroundHoverColor = ToByteColor(ButtonUi::HoverColor);
  mBackgroundClickedColor = ToByteColor(ButtonUi::ClickedColor);
}

void ButtonBase::SetCommand(Command* command)
{
  if(mCommand)
    mCommand->GetDispatcher()->Disconnect(this);

  mCommand = command;
  mToolTipText = command->ToolTip;
  ConnectThisTo(command, Events::CommandStateChange, OnCommandStateChange);
}

void ButtonBase::UpdateTransform()
{
  mBackground->SetSize(mSize);
  mBorder->SetSize(mSize);
  mFocusBorder->SetSize(mSize);

  mFocusBorder->SetColor(ToFloatColor(mFocusBorderColor));
  mFocusBorder->SetVisible(HasFocus());
  mBorder->SetColor(ToFloatColor(mBorderColor));

  if(mMouseDown)
    mBackground->SetColor(ToFloatColor(mBackgroundClickedColor));
  else if(mMouseOver)
    mBackground->SetColor(ToFloatColor(mBackgroundHoverColor));
  else
    mBackground->SetColor(ToFloatColor(mBackgroundColor));

  Composite::UpdateTransform();
}

void ButtonBase::OnMouseEnter(MouseEvent* event)
{
  if(mIgnoreInput)
    return;
  mMouseOver = true;
  MarkAsNeedsUpdate();
}

void ButtonBase::OnMouseExit(MouseEvent* event)
{
  mShowToolTip = true;
  if(mIgnoreInput)
    return;
  mMouseOver = false;
  mMouseDown = false;
  MarkAsNeedsUpdate();
}

void ButtonBase::OnMouseDown(MouseEvent* event)
{
  event->Handled = true;
  if(mIgnoreInput)
    return;
  mMouseDown = true;
  MarkAsNeedsUpdate();
}

void ButtonBase::OnMouseUp(MouseEvent* event)
{
  event->Handled = true;
  if(mIgnoreInput)
    return;
  mMouseDown = false;
  MarkAsNeedsUpdate();
}

void ButtonBase::OnHover(MouseEvent* event)
{
  if(mShowToolTip && !mToolTipText.Empty())
  {
    ToolTip* toolTip = new ToolTip(this);
    toolTip->SetText(mToolTipText);
    toolTip->SetColor(mToolTipColor);

    ToolTipPlacement placement;
    placement.SetScreenRect(GetScreenRect());
    placement.SetPriority(IndicatorSide::Bottom, IndicatorSide::Right, 
                          IndicatorSide::Left, IndicatorSide::Top);
    toolTip->SetArrowTipTranslation(placement);
    // Temporary to fix jitter when the tool pops up
    toolTip->UpdateTransform();
    mToolTip = toolTip;
  }
}

void ButtonBase::OnCommandStateChange(ObjectEvent* event)
{
  MarkAsNeedsUpdate();
}

void ButtonBase::OnMouseClick(MouseEvent* event)
{
  if(mIgnoreInput)
    return;

  event->Handled = true;
  Activate();
}

bool ButtonBase::TakeFocusOverride()
{
  if(!mTabFocusStop)
    return false;

  this->HardTakeFocus();
  return true;
}

void ButtonBase::SetIgnoreInput(bool state)
{
  if(mIgnoreInput == state)
    return;

  mIgnoreInput = state;
  mMouseDown = false;
  mMouseOver = false;
  MarkAsNeedsUpdate();
}

void ButtonBase::OnKeyDown(KeyboardEvent* event)
{
  if(!mIgnoreInput && event->Key == Keys::Space || event->Key == Keys::Enter)
    Activate();

  TabJump(this, event);
}

void ButtonBase::OnFocusGained(FocusEvent* event)
{
  MarkAsNeedsUpdate();
}

void ButtonBase::Activate()
{
  mToolTip.SafeDestroy();
  mShowToolTip = false;

  ObjectEvent e(this);
  GetDispatcher()->Dispatch(Events::ButtonPressed, &e);
  if(mCommand)
  {
    CommandCaptureContextEvent commandCaptureEvent;
    commandCaptureEvent.ActiveSet = CommandManager::GetInstance();
    this->DispatchBubble(Events::CommandCaptureContext, &commandCaptureEvent);
    mCommand->Execute();
  }
}

//------------------------------------------------------------------ Text Button
ZilchDefineType(TextButton, builder, type)
{

}

//******************************************************************************
TextButton::TextButton(Composite* parent, StringParam textStyle)
  : ButtonBase(parent, "TextButton")
{
  mButtonText = new Text(this, textStyle);
  
  mStyle = TextButtonStyle::Classic;

  mTextColor = ToByteColor(TextBUttonUi::TextColor);
  mTextHoverColor = ToByteColor(TextBUttonUi::TextHoverColor);
  mTextClickedColor = ToByteColor(TextBUttonUi::TextClickedColor);
}

TextButton::TextButton(Composite* parent, StringParam fontName, uint fontSize)
  : ButtonBase(parent, "TextButton")
{
  mButtonText = new Text(this, fontName, fontSize);

  mStyle = TextButtonStyle::Classic;

  mTextColor = ToByteColor(TextBUttonUi::TextColor);
  mTextHoverColor = ToByteColor(TextBUttonUi::TextHoverColor);
  mTextClickedColor = ToByteColor(TextBUttonUi::TextClickedColor);
}

//******************************************************************************
void TextButton::SetStyle(TextButtonStyle::Type style)
{
  mStyle = style;

  // Hide the background and border if it's a modern style
  bool classic = (mStyle == TextButtonStyle::Classic);
  mBackground->SetVisible(classic);
  mBorder->SetVisible(classic);

  MarkAsNeedsUpdate();
}

//******************************************************************************
void TextButton::SetText(StringParam newValue)
{
  mButtonText->SetText(newValue);
  mButtonText->SizeToContents();
}

//******************************************************************************
void TextButton::UpdateTransform()
{
  /// Only update the text color in the modern style
  if(mStyle == TextButtonStyle::Modern)
  {
    if(mMouseDown)
      mButtonText->SetColor(ToFloatColor(mTextClickedColor));
    else if(mMouseOver)
      mButtonText->SetColor(ToFloatColor(mTextHoverColor));
    else
      mButtonText->SetColor(ToFloatColor(mTextColor));
  }

  // Even though the button base sets the size of the background, we
  // need it set now so we get correct padding from it
  mBackground->SetSize(mSize);

  // Place the button text
  Thickness padding = GetPadding();
  Rect rect = RemoveThicknessRect(padding, mSize);

  const Vec2 cOffsets[2] = {Pixels(0, -1), Vec2::cZero};

  PlaceCenterToRect(rect, mButtonText, cOffsets[mStyle]);

  ButtonBase::UpdateTransform();
}

//******************************************************************************
Vec2 TextButton::GetMinSize()
{
  Vec2 size = mButtonText->GetMinSize();
  Thickness padding = GetPadding();
  return Math::Max(mMinSize, ExpandSizeByThickness(padding, size));
}

//******************************************************************************
Thickness TextButton::GetPadding()
{
  const Thickness cButtonPadding(4,4,4,4);

  Thickness padding = Thickness::cZero;

  // Add extra padding for the classic style
  if(mStyle == TextButtonStyle::Classic)
  {
    padding = mBackground->GetBorderThickness();
    padding = padding + cButtonPadding;
  }

  return padding;
}

//------------------------------------------------------------------ Icon Button
ZilchDefineType(IconButton, builder, type)
{

}

IconButton::IconButton(Composite* parent)
  : ButtonBase(parent, "TextButton")
{
  mIcon = NULL;
  mPadding = Thickness(IconButtonUi::Padding);

  mIconColor = mIconHoverColor = mIconClickedColor = Color::White;

  mBackgroundColor = ToByteColor(IconButtonUi::DefaultColor);
  mBackgroundHoverColor = ToByteColor(IconButtonUi::HoverColor);
  mBackgroundClickedColor = ToByteColor(IconButtonUi::ClickedColor);
}

void IconButton::SetCommand(Command* command)
{
  ButtonBase::SetCommand(command);
  SetIcon(command->IconName);
}

void IconButton::UpdateIconColor()
{
  if(mMouseDown)
    mIcon->SetColor(ToFloatColor(mIconClickedColor));
  else if(mMouseOver)
    mIcon->SetColor(ToFloatColor(mIconHoverColor));
  else
    mIcon->SetColor(ToFloatColor(mIconColor));
}

void IconButton::SetIcon(StringParam newValue)
{
  SafeDestroy(mIcon);
  mIcon = CreateAttached<Element>(newValue);
  ErrorIf(mIcon == NULL, "Icon not found.");
  MarkAsNeedsUpdate(true);
}

void IconButton::SetIconColor(Vec4Param color)
{
  if(mIcon)
    mIcon->SetColor(color);
}

Vec2 IconButton::GetMinSize()
{
  return ExpandSizeByThickness(mPadding, mIcon->GetMinSize());
}

void IconButton::UpdateTransform()
{
  mBackground->SetSize(mSize);
  Rect rect = RemoveThicknessRect(mPadding, mSize);

  if(mIcon)
    PlaceCenterToRect(rect, mIcon);

  // Call this before setting colors as we're going to overwrite the colors
  ButtonBase::UpdateTransform();

  // Command can show themselves as active
  bool commandActive = mCommand && mCommand->IsActive();

  if(mMouseDown)
  {
    mBackground->SetColor(ToFloatColor(mBackgroundClickedColor));
  }
  else if(mMouseOver)
  {
    if(commandActive)
      mBackground->SetColor(IconButtonUi::ActiveHoverColor);
    else
      mBackground->SetColor(ToFloatColor(mBackgroundHoverColor));
  }
  else
  {
    if(commandActive)
      mBackground->SetColor(IconButtonUi::ActiveColor);
    else
      mBackground->SetColor(ToFloatColor(mBackgroundColor));
  }

  UpdateIconColor();

  Composite::UpdateTransform();
}

void IconButton::OnMouseEnter(MouseEvent* e)
{
  ButtonBase::OnMouseEnter(e);
  UpdateIconColor();
}

void IconButton::OnMouseExit(MouseEvent* e)
{
  ButtonBase::OnMouseExit(e);
  UpdateIconColor();
}

//----------------------------------------------------------- Toggle Icon Button
ZilchDefineType(ToggleIconButton, builder, type)
{

}

ToggleIconButton::ToggleIconButton(Composite* parent)
 : IconButton(parent)
{
  ConnectThisTo(this, Events::ButtonPressed, OnButtonPressed);

  mEnabled = true;
  mIgnoreClicks = false;
  mIconDisabledColor = mIconDisabledHoverColor = Color::White;
}

void ToggleIconButton::SetEnabledIcon(StringParam newValue)
{
  mEnabledIcon = newValue;
  if(mIcon == NULL && mEnabled)
    SetIcon(mEnabledIcon);
}

void ToggleIconButton::SetDisabledIcon(StringParam newValue)
{
  mDisabledIcon = newValue;
  if(mIcon == NULL && !mEnabled)
    SetIcon(mEnabledIcon);
}

void ToggleIconButton::SetEnabled(bool state)
{
  mEnabled = state;
  UpdateIcon();
}

bool ToggleIconButton::GetEnabled()
{
  return mEnabled;
}

void ToggleIconButton::UpdateIconColor()
{
  if(mMouseOver)
  {
    if(mEnabled)
      mIcon->SetColor(ToFloatColor(mIconHoverColor));
    else
      mIcon->SetColor(ToFloatColor(mIconDisabledHoverColor));
  }
  else
  {
    if(mEnabled)
      mIcon->SetColor(ToFloatColor(mIconColor));
    else
      mIcon->SetColor(ToFloatColor(mIconDisabledColor));
  }
}

void ToggleIconButton::UpdateIcon()
{
  if(mEnabled)
    SetIcon(mEnabledIcon);
  else
    SetIcon(mDisabledIcon);

  UpdateIconColor();
}

void ToggleIconButton::OnButtonPressed(Event* e)
{
  if(mIgnoreClicks)
    return;

  mEnabled = !mEnabled;
  UpdateIcon();
}

}//namespace Zero
