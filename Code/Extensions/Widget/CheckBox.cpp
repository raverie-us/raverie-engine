// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

const String cCheckBoxIcon = "Background";
const String cCheckIcon = "Icon";
const String cCheckInvalidIcon = "InvalidIcon";

namespace CheckBoxUi
{
const cstr cLocation = "EditorUi/Controls/CheckBox";
Tweakable(Vec4, BackgroundColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, BorderColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, CheckColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, ReadOnlyCheckColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, FocusBorderColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec2, CheckSize, Vec2(12, 12), cLocation);
} // namespace CheckBoxUi

RaverieDefineType(CheckBox, builder, type)
{
}

CheckBox::CheckBox(Composite* parent) : Composite(parent)
{
  static const String className = "CheckBox";
  mDefSet = mDefSet->GetDefinitionSet(className);

  mChecked = false;
  mAllowEdit = true;
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBorder = CreateAttached<Element>(cWhiteSquareBorder);
  mCheckIcon = CreateAttached<Element>(cCheckIcon);

  ConnectThisTo(this, Events::LeftClick, OnLeftClick);
  ConnectThisTo(this, Events::KeyDown, OnKeyDown);

  ConnectThisTo(this, Events::FocusGained, OnFocusGained);
  ConnectThisTo(this, Events::FocusLost, OnFocusGained);

  SetCheckedDirect(false);

  mCheckIcon->SetColor(CheckBoxUi::CheckColor);
  mBackground->SetColor(CheckBoxUi::BackgroundColor);
}

CheckBox::~CheckBox()
{
}

void CheckBox::SetInvalid()
{
  mCheckIcon->SetVisible(true);
  mCheckIcon->ChangeDefinition(mDefSet->GetDefinition(cCheckInvalidIcon));
}

void CheckBox::SetEditable(bool editable)
{
  mAllowEdit = editable;
  if (editable)
    mCheckIcon->SetColor(CheckBoxUi::CheckColor);
  else
    mCheckIcon->SetColor(CheckBoxUi::ReadOnlyCheckColor);

  mBorder->SetVisible(editable);
}

bool CheckBox::GetEditable()
{
  return mAllowEdit;
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
  if (mChecked)
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
  if (!mAllowEdit)
    return false;

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

  if (HasFocus())
    mBorder->SetColor(CheckBoxUi::FocusBorderColor);
  else
    mBorder->SetColor(CheckBoxUi::BorderColor);

  Composite::UpdateTransform();
}

void CheckBox::OnKeyDown(KeyboardEvent* event)
{
  if (event->Key == Keys::Space || event->Key == Keys::Enter)
  {
    if (!event->GetModifierPressed())
      ToggleChecked();
  }

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

RaverieDefineType(TextCheckBox, builder, type)
{
}

TextCheckBox::TextCheckBox(Composite* parent) : Composite(parent)
{
  this->SetLayout(CreateRowLayout());
  mCheckBox = new CheckBox(this);
  mText = new Label(this);

  ConnectThisTo(mText, Events::LeftClick, OnLeftClick);
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

void TextCheckBox::OnLeftClick(MouseEvent* event)
{
  mCheckBox->ToggleChecked();
}

} // namespace Raverie
