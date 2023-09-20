// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

namespace Events
{
// The button has been pressed.
DeclareEvent(ButtonPressed);
} // namespace Events
class Command;

class ButtonBase : public Composite
{
public:
  RaverieDeclareType(ButtonBase, TypeCopyMode::ReferenceType);

  ButtonBase(Composite* parent, StringParam styleClass);

  void SetToolTip(StringParam text);

  // ButtonBase Interface
  virtual void AddCommand(Command* command);
  virtual void OnCommandStateChange(ObjectEvent* event);
  void UpdateTransform() override;
  bool TakeFocusOverride() override;

  void SetIgnoreInput(bool state);

  Element* mBackground;
  Element* mBorder;
  Element* mFocusBorder;

  /// Border colors.
  ByteColor mBorderColor, mFocusBorderColor;
  ByteColor mBackgroundColor, mBackgroundHoverColor, mBackgroundClickedColor;

  void Activate();
  // Events
  virtual void OnMouseEnter(MouseEvent* event);
  virtual void OnMouseExit(MouseEvent* event);
  void OnMouseDown(MouseEvent* event);
  void OnMouseUp(MouseEvent* event);
  void OnHover(MouseEvent* event);
  void OnKeyDown(KeyboardEvent* event);
  void OnFocusGained(FocusEvent* event);
  void OnMouseClick(MouseEvent* event);

  bool mIgnoreInput;
  bool mMouseDown;
  bool mMouseOver;
  bool mTabFocusStop;
  Array<Command*> mCommands;

  /// When the button is pressed, we don't want the tooltip to be shown again
  /// until the mouse has exited and re-entered the button. This helps protect
  /// against the user pressing the button, Ui popping up below, and then
  /// the tooltip popping up on top of the new Ui.
  bool mShowToolTip;
  String mToolTipText;
  ToolTipColorScheme::Enum mToolTipColor;
  HandleOf<Widget> mToolTip;
};

DeclareEnum2(TextButtonStyle, Classic, Modern);
/// Text button is a basic button with text.
class TextButton : public ButtonBase
{
public:
  RaverieDeclareType(TextButton, TypeCopyMode::ReferenceType);

  /// Constructor.
  TextButton(Composite* parent, StringParam textStyle = cText);
  TextButton(Composite* parent, StringParam fontName, uint fontSize);

  /// Sets the display style of the button.
  void SetStyle(TextButtonStyle::Type style);

  /// Set the text on the button.
  void SetText(StringParam newValue);

  /// Widget Interface
  void UpdateTransform() override;
  Vec2 GetMinSize() override;

  Thickness GetPadding();

  /// Text colors.
  Vec4 mTextColor, mTextHoverColor, mTextClickedColor;

  /// The current style of the button.
  TextButtonStyle::Type mStyle;

  /// The buttons text.
  Text* mButtonText;
};

/// Icon button is a button with a icon for tool bars.
class IconButton : public ButtonBase
{
public:
  RaverieDeclareType(IconButton, TypeCopyMode::ReferenceType);

  IconButton(Composite* parent);

  /// Set the icon on the button.
  void SetIcon(StringParam newValue);
  void SetIconColor(Vec4Param color);

  /// Composite Interface.
  void UpdateTransform() override;
  Vec2 GetMinSize() override;

  // ButtonBase Interface
  void AddCommand(Command* command) override;

  virtual void UpdateIconColor();

  Thickness mPadding;

  ByteColor mIconColor, mIconHoverColor, mIconClickedColor;

protected:
  /// Mouse Response.
  void OnMouseEnter(MouseEvent* e) override;
  void OnMouseExit(MouseEvent* e) override;

  Element* mIcon;
};

/// Used for switching between two icons when clicked
class ToggleIconButton : public IconButton
{
public:
  RaverieDeclareType(ToggleIconButton, TypeCopyMode::ReferenceType);

  ToggleIconButton(Composite* parent);

  void SetEnabledIcon(StringParam newValue);
  void SetDisabledIcon(StringParam newValue);

  void SetEnabled(bool state);
  bool GetEnabled();

  void UpdateIconColor() override;

  /// Used to disable event response for when it's clicked.
  bool mIgnoreClicks;

  ByteColor mIconDisabledColor, mIconDisabledHoverColor;

private:
  void UpdateIcon();

  // Switch the icon of the button.
  void OnButtonPressed(Event* e);

  bool mEnabled;
  String mEnabledIcon, mDisabledIcon;
};

namespace IconButtonUi
{
DeclareTweakable(Vec4, DefaultColor);
}

} // namespace Raverie
