// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Basic Check Box
class CheckBox : public Composite
{
public:
  RaverieDeclareType(CheckBox, TypeCopyMode::ReferenceType);

  CheckBox(Composite* parent);
  ~CheckBox();

  void SetInvalid();

  // Is the check box editable?
  void SetEditable(bool editable);
  bool GetEditable();

  /// Is the check box checked
  void SetChecked(bool value);
  bool GetChecked();

  /// Toggle the Checked State
  void ToggleChecked();

  /// Set checked without sending ValueChanged message
  void SetCheckedDirect(bool value);

  // Widget Interface
  void UpdateTransform() override;
  bool TakeFocusOverride() override;
  Vec2 GetMinSize() override;

private:
  void OnLeftClick(MouseEvent* event);
  void OnKeyDown(KeyboardEvent* event);
  void OnFocusGained(FocusEvent* event);

  bool mAllowEdit;

  Element* mCheckIcon;
  Element* mBorder;
  Element* mBackground;
  bool mChecked;
};

// Check Box with text label next to the control
class TextCheckBox : public Composite
{
public:
  RaverieDeclareType(TextCheckBox, TypeCopyMode::ReferenceType);

  TextCheckBox(Composite* parent);
  ~TextCheckBox();

  /// Is the check box checked
  bool GetChecked();
  void SetChecked(bool value);

  /// Text for check box.
  String GetText();
  void SetText(StringParam text);

  /// Toggle the Checked State
  void ToggleChecked();

  /// Set checked without sending ValueChanged message
  void SetCheckedDirect(bool value);

  void OnLeftClick(MouseEvent* event);

  // private:
  Label* mText;
  CheckBox* mCheckBox;
};

} // namespace Raverie
