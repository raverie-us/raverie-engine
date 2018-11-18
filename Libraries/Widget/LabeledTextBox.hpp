///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// A text box with a label to the left. The text box will take up
/// all of the remaining space after the label is sized to its text.
class LabeledTextBox : public Composite
{
public:
  typedef LabeledTextBox ZilchSelf;

  LabeledTextBox(Composite* parent);

  /// The text of the label
  String GetLabelText() const;
  void SetLabelText(StringParam labelText);

  /// The text of the text box
  String GetText() const;
  void SetText(StringParam text);

  TextBox* mTextBox;
  Label* mLabel;
};

}//namespace Zero
