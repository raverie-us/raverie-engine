///////////////////////////////////////////////////////////////////////////////
///
/// \file TextBox.cpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace TextBoxUi
{
  const cstr cLocation = "EditorUi/Controls/TextBox";
  Tweakable(Vec4, BackgroundColor,         Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BorderColor,             Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, FocusBorderColor,        Vec4(1,1,1,1), cLocation);
  // The color of the background element when the text box is read only
  Tweakable(Vec4, ReadOnlyBackgroundColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, ReadOnlyTextColor,       Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, HintColor,               Vec4(1,1,1,1), cLocation);
}

namespace ModernTextBoxUi
{
  const cstr cLocation = "EditorUi/Controls/TextBox/Modern";
  Tweakable(Vec4, BackgroundColor,         Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BorderColor,             Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, FocusBorderColor,        Vec4(1,1,1,1), cLocation);
  // The color of the background element when the text box is read only
  Tweakable(Vec4, ReadOnlyBackgroundColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, ReadOnlyTextColor,       Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, HintColor,               Vec4(1,1,1,1), cLocation);
}

namespace TextBoxButtonUi
{
  const cstr cLocation = "EditorUi/Controls/TextBox/TextBoxButton";
  Tweakable(Vec4, BackgroundColor,   Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BackgroundHover,   Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BackgroundClicked, Vec4(1,1,1,1), cLocation);
  Tweakable(float, ButtonWidth,      34.0f,         cLocation);
}

//------------------------------------------------------------- Text Box Control
ZilchDefineType(TextBox, builder, type)
{
}

static const String TextBoxClass = "TextBox";

const Thickness TextBoxPadding = Thickness(2,2,2,2);

TextBox::TextBox(Composite* parent, StringParam styleName)
  : Composite(parent)
{
  String style = styleName;
  if(style.Empty())
    style = TextBoxClass;

  mDefSet = mDefSet->GetDefinitionSet(style);

  mTextOffset = 0.0f;
  mAllowEdit = true;
  mMeasureForMinSize = true;

  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(TextBoxUi::BackgroundColor);
  mBorder = CreateAttached<Element>(cWhiteSquareBorder);
  mBorder->SetInteractive(false);

  Thickness borderThickness = TextBoxPadding;
  mEditTextField = new EditText(this);
  mEditTextField->SetTranslation(Vec3(borderThickness.TopLeft()));

  mHint = new Text(this, cText);
  mHint->SetVisible(false);
  mHint->SetInteractive(false);

  SetStyle(TextBoxStyle::Classic);

  ConnectThisTo(mEditTextField, Events::MouseEnter, MouseEnter);
  ConnectThisTo(mEditTextField, Events::MouseExit, MouseExit);
  ConnectThisTo(mEditTextField, Events::TextSubmit, OnSubmit);
  ConnectThisTo(mEditTextField, Events::KeyDown, OnKeyDown);
  ConnectThisTo(mEditTextField, Events::KeyUp, OnKeyUp);
  ConnectThisTo(this, Events::FocusGained, OnFocusGained);
  ConnectThisTo(this, Events::FocusLost, OnFocusLost);
}

void TextBox::SizeToContents()
{
  Thickness thickness = mBackground->GetBorderThickness();
  mSize = ExpandSizeByThickness(thickness, mEditTextField->GetMinSize());
  this->MarkAsNeedsUpdate();
}

TextBox::~TextBox()
{
  //
}

void TextBox::SetReadOnly(bool state)
{
  SetEditable(!state);
  if(state)
  {
    mBackground->SetColor(TextBoxUi::ReadOnlyBackgroundColor);
    mEditTextField->SetColor(TextBoxUi::ReadOnlyTextColor);
  }
  else
  {
    mBackground->SetColor(TextBoxUi::BackgroundColor);
    mEditTextField->SetColor(Vec4(1,1,1,1));
  }

  mBorder->SetVisible(!state);
}

void TextBox::SetInvalid()
{
  mEditTextField->SetText("-");
}

void TextBox::OnSubmit(ObjectEvent* event)
{
  ObjectEvent objectEvent(this);
  GetDispatcher()->Dispatch(Events::TextBoxChanged, &objectEvent);
}

void TextBox::SetEditable(bool editable)
{
  mAllowEdit = editable;
  mEditTextField->SetEditable(editable);
  if(editable == false)
    LoseFocus();
}


bool TextBox::TakeFocusOverride()
{
  if(!mAllowEdit)
    return false;
  mEditTextField->HardTakeFocus();
  mEditTextField->SelectAll();
  return true;
}

Vec2 TextBox::GetMinSize()
{
  if(mMeasureForMinSize)
  {
    Vec2 textSize = mEditTextField->GetMinSize();
    Thickness thickness = TextBoxPadding;
    return ExpandSizeByThickness(thickness, textSize);
  }
  else
  {
    return Composite::GetMinSize();
  }
}

void TextBox::SetTextOffset(float offset)
{
  mEditTextField->SetTextOffset(offset);
}

bool TextBox::GetEditable()
{
  return mAllowEdit;
}

void TextBox::SetText(StringParam text)
{
  mEditTextField->SetText(text);
}

String TextBox::GetText()
{
  return mEditTextField->GetText();
}

void TextBox::SetStyle(TextBoxStyle::Type style)
{
  if(style == TextBoxStyle::Classic)
  {
    mBackgroundColor = ToByteColor(TextBoxUi::BackgroundColor);
    mBorderColor = ToByteColor(TextBoxUi::BorderColor);
    mFocusBorderColor = ToByteColor(TextBoxUi::FocusBorderColor);
  }
  else
  {
    mBackgroundColor = ToByteColor(ModernTextBoxUi::BackgroundColor);
    mBorderColor = ToByteColor(ModernTextBoxUi::BorderColor);
    mFocusBorderColor = ToByteColor(ModernTextBoxUi::FocusBorderColor);
  }

  MarkAsNeedsUpdate();
}

void TextBox::SetHintText(StringParam text)
{
  mHint->SetText(text);
}

void TextBox::MouseEnter(MouseEvent* event)
{
  if(mAllowEdit)
    event->GetMouse()->SetCursor(Cursor::TextBeam);
}

void TextBox::MouseExit(MouseEvent* event)
{
  event->GetMouse()->SetCursor(Cursor::Arrow);
}

void TextBox::UpdateTransform()
{
  Thickness borderThickness = TextBoxPadding;
  mBackground->SetSize(mSize);
  mBackground->SetColor(ToFloatColor(mBackgroundColor));

  mBorder->SetSize(mSize);
  if(HasFocus())
    mBorder->SetColor(ToFloatColor(mFocusBorderColor));
  else
    mBorder->SetColor(ToFloatColor(mBorderColor));

  LayoutResult lr = RemoveThickness(borderThickness, mSize - Vec2(mTextOffset, 0), Vec3(mTextOffset,0,0));
  lr.Translation.y = SnapToPixels(lr.Translation.y);
  mEditTextField->SetTranslation(lr.Translation);
  mEditTextField->SetSize(lr.Size);

  mHint->SetColor(TextBoxUi::HintColor);
  if(!HasFocus() && GetText().Empty())
  {
    mHint->SetTranslation(lr.Translation);
    mHint->SetSize(lr.Size);
    mHint->SetVisible(true);
  }
  else
  {
    mHint->SetVisible(false);
  }

  Composite::UpdateTransform();
}

void TextBox::HideBackground(bool value)
{
  mBackground->SetVisible(!value);
  mBorder->SetVisible(!value);
}

void TextBox::SetTextClipping(bool value)
{
  mEditTextField->SetTextClipping(value);
}

void TextBox::OnKeyDown(KeyboardEvent* event)
{
  MarkAsNeedsUpdate();
}

void TextBox::OnKeyUp(KeyboardEvent* event)
{
}

void TextBox::OnFocusGained(Event*)
{
  mBorder->SetColor(ToFloatColor(mFocusBorderColor));
}

void TextBox::OnFocusLost(Event*)
{
  mBorder->SetColor(ToFloatColor(mBorderColor));
}

void TextBox::SetPassword(bool passwordMode)
{
  mEditTextField->mPassword = passwordMode;
}

//---------------------------------------------------------------- TextBoxButton
//******************************************************************************
TextBoxButton::TextBoxButton(Composite* parent, StringParam iconName) :
  TextBox(parent)
{
  mButton = new IconButton(this);
  mButton->SetIcon(iconName);
  mButton->mBackgroundColor = ToByteColor(TextBoxButtonUi::BackgroundColor);
  mButton->mBackgroundHoverColor = ToByteColor(TextBoxButtonUi::BackgroundHover);
  mButton->mBackgroundClickedColor = ToByteColor(TextBoxButtonUi::BackgroundClicked);
  mButton->mBorder->SetVisible(false);
}

//******************************************************************************
void TextBoxButton::UpdateTransform()
{
  Vec3 pos = Vec3::cZero;
  pos.x = mSize.x - mButton->GetSize().x - Pixels(1);
  pos.y = Pixels(1);
  mButton->SetTranslation(pos);

  mButton->SetSize(Vec2(TextBoxButtonUi::ButtonWidth, mSize.y - Pixels(2)));

  TextBox::UpdateTransform();
}

//---------------------------------------------------------------- MultiLineText
MultiLineText::MultiLineText(Composite* parent, StringParam textStyle)
  :Composite(parent)
{
  String style = textStyle;
  if(style.Empty())
    style = cText;

  Text* text = new Text(this, style);
  Initialize(text);
}

MultiLineText::MultiLineText(Composite* parent, StringParam font, uint fontSize) :
  Composite(parent)
{
  Text* text = new Text(this, font, fontSize);
  Initialize(text);
}

Thickness MultiLineTextPadding = Thickness(2,2,2,2);

Vec2 MultiLineText::GetMinSize()
{
  Vec2 minSize = mTextField->GetBoundedSize(mMaxLineWidth, 10000);
  return ExpandSizeByThickness(MultiLineTextPadding, minSize);
}

void MultiLineText::UpdateTransform()
{
  mBackground->SetSize(mSize);
  mBorder->SetSize(mSize);

  Rect rect = RemoveThicknessRect(MultiLineTextPadding, mSize);
  PlaceWithRect(rect, mTextField);

  Composite::UpdateTransform();
}

void MultiLineText::SetText(StringParam text)
{
  mTextField->SetText(text);
}

String MultiLineText::GetText()
{
  return mTextField->GetText();
}

void MultiLineText::Initialize(Text* textObject)
{
  mDefSet = mDefSet->GetDefinitionSet(TextBoxClass);

  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBorder = CreateAttached<Element>(cWhiteSquareBorder);
  mBorder->SetInteractive(false);

  // Move these behind the text object
  mBorder->MoveToBack();
  mBackground->MoveToBack();

  Thickness borderThickness = mBackground->GetBorderThickness();

  mTextField = textObject;
  mTextField->SetTranslation(Vec3(borderThickness.TopLeft()));
  mTextField->SetMultiLine(true);

  mBackground->SetColor(TextBoxUi::BackgroundColor);
  mBorder->SetColor(TextBoxUi::BorderColor);

  mMaxLineWidth = 300;
}

}
