// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

LabeledTextBox::LabeledTextBox(Composite* parent) : Composite(parent)
{
  SetLayout(CreateRowLayout());
  SetClipping(true);

  mLabel = new Label(this);
  mLabel->SetSizing(SizeAxis::X, SizePolicy::Auto, 0);

  mTextBox = new TextBox(this);
  mTextBox->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
  mTextBox->SetEditable(true);
}

String LabeledTextBox::GetLabelText() const
{
  return mLabel->GetText();
}

void LabeledTextBox::SetLabelText(StringParam labelText)
{
  mLabel->SetText(labelText);
  mLabel->SizeToContents();
  MarkAsNeedsUpdate();
}

String LabeledTextBox::GetText() const
{
  return mTextBox->GetText();
}

void LabeledTextBox::SetText(StringParam text)
{
  mTextBox->SetText(text);
  MarkAsNeedsUpdate();
}

} // namespace Raverie
