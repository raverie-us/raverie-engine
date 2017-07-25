///////////////////////////////////////////////////////////////////////////////
///
/// \file CheckBox.cpp
///  
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------- Check Box
const String cCheckBoxIcon = "Background";
const String cCheckIcon = "Icon";
const String cCheckInvalidIcon = "InvalidIcon";

namespace CheckBoxUi
{
  const cstr cLocation = "EditorUi/Controls/CheckBox";
  Tweakable(Vec4, BackgroundColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BorderColor,     Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, CheckColor,      Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, FocusBorderColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec2, CheckSize, Vec2(12,12), cLocation);
}

ZilchDefineType(CheckBox, builder, type)
{
}

CheckBox::CheckBox(Composite* parent)
  : Composite(parent)
{
  static const String className = "CheckBox";
  mDefSet = mDefSet->GetDefinitionSet(className);

  mChecked = false;
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBorder = CreateAttached<Element>(cWhiteSquareBorder);
  mCheckIcon = CreateAttached<Element>(cCheckIcon);

  ConnectThisTo(this, Events::LeftClick, OnLeftClick);
  ConnectThisTo(this, Events::KeyDown, OnKeyDown);

  ConnectThisTo(this, Events::FocusGained, OnFocusGained);
  ConnectThisTo(this, Events::FocusLost, OnFocusGained);

  SetCheckedDirect(false);
}

CheckBox::~CheckBox()
{

}

void CheckBox::SetInvalid()
{
  mCheckIcon->SetVisible(true);
  mCheckIcon->ChangeDefinition(mDefSet->GetDefinition(cCheckInvalidIcon));
}

bool CheckBox::GetChecked()
{
  return mChecked;
}

void CheckBox::SetChecked(bool value)
{
  SetCheckedDirect(value);
  ObjectEvent e(this);
  GetDispatcher()->Dispatch(Events::ValueChanged, &e);
}

void CheckBox::SetCheckedDirect(bool value)
{
  mChecked = value;
  if(mChecked)
  {
    mCheckIcon->SetVisible(true);
    mCheckIcon->ChangeDefinition(mDefSet->GetDefinition(cCheckIcon));
  }
  else
  {
    mCheckIcon->SetVisible(false);
  }
}

void CheckBox::ToggleChecked()
{
  SetChecked(!GetChecked());
}

bool CheckBox::TakeFocusOverride()
{
  this->HardTakeFocus();
  return true;
}

Vec2 CheckBox::GetMinSize()
{
  return CheckBoxUi::CheckSize;
}

void CheckBox::UpdateTransform()
{
  Vec2 boxSize = CheckBoxUi::CheckSize;
  Vec2 checkSize = mCheckIcon->GetSize();
  Vec3 boxOffset = GetCenterPosition(mSize, boxSize);
  Vec3 checkOffset = GetCenterPosition(boxSize, checkSize);

  mCheckIcon->SetTranslation(boxOffset + checkOffset);

  mBorder->SetTranslation(boxOffset);
  mBorder->SetSize(boxSize);

  mBackground->SetTranslation(boxOffset);
  mBackground->SetSize(boxSize);

  mCheckIcon->SetColor(CheckBoxUi::CheckColor);

  if(HasFocus())
    mBorder->SetColor(CheckBoxUi::FocusBorderColor);
  else
    mBorder->SetColor(CheckBoxUi::BorderColor);

  mBackground->SetColor(CheckBoxUi::BackgroundColor);

  Composite::UpdateTransform();
}

void CheckBox::OnKeyDown(KeyboardEvent* event)
{
  if(event->Key == Keys::Space || event->Key == Keys::Enter)
    ToggleChecked();

  TabJump(this, event);
}

void CheckBox::OnFocusGained(FocusEvent* event)
{
  MarkAsNeedsUpdate();
}

void CheckBox::OnLeftClick(MouseEvent* event)
{
  ToggleChecked();
}

//--------------------------------------------------------------- Text Check Box
ZilchDefineType(TextCheckBox, builder, type)
{
}

TextCheckBox::TextCheckBox(Composite* parent) : Composite(parent)
{
  this->SetLayout(CreateRowLayout());
  mCheckBox = new CheckBox(this);
  mText = new Label(this);
}

TextCheckBox::~TextCheckBox()
{
}

bool TextCheckBox::GetChecked()
{
  return mCheckBox->GetChecked();
}

void TextCheckBox::SetChecked(bool value)
{
  mCheckBox->SetChecked(value);
}

void TextCheckBox::SetCheckedDirect(bool value)
{
  mCheckBox->SetCheckedDirect(value);
}

void TextCheckBox::ToggleChecked()
{
  mCheckBox->ToggleChecked();
}

String TextCheckBox::GetText()
{
  return mText->GetText();
}

void TextCheckBox::SetText(StringParam text)
{
  mText->SetText(text);
  mText->SizeToContents();
  mSize = mText->GetSize();
}

}
