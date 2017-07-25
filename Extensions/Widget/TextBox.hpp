///////////////////////////////////////////////////////////////////////////////
///
/// \file TextBox.hpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Text;
class EditText;

//--------------------------------------------------------------------- Text Box
DeclareEnum2(TextBoxStyle, Classic, Modern);

/// Text box is a simple edit text box with a background.
class TextBox : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  TextBox(Composite* parent, StringParam style = String());
  ~TextBox();

  void SetReadOnly(bool state);
  void SetInvalid();

  //Is the text box editable?
  void SetEditable(bool editable);
  bool GetEditable();

  //Set the text on the text box.
  void SetText(StringParam text);
  String GetText();

  /// Sets the display style of the text box.
  void SetStyle(TextBoxStyle::Type style);

  void SetHintText(StringParam text);
  void HideBackground(bool value);
  void SetTextClipping(bool value);
  void SetPassword(bool passwordMode);

  //Widget interfaces
  void UpdateTransform() override;
  void SizeToContents() override;
  bool TakeFocusOverride() override;

  Vec2 GetMinSize();

  void SetTextOffset(float offset);

  EditText* mEditTextField;

  float mTextOffset;

  TextBoxStyle::Type mStyle;

  ByteColor mBackgroundColor, mBorderColor, mFocusBorderColor;

  bool mMeasureForMinSize;

private:
  //Events
  void OnSubmit(ObjectEvent* event);
  void MouseEnter(MouseEvent* event);
  void MouseExit(MouseEvent* event);
  void OnKeyDown(KeyboardEvent* event);
  void OnKeyUp(KeyboardEvent* event);
  void OnFocusGained(Event*);
  void OnFocusLost(Event*);
  
  bool mAllowEdit;
  
  Text* mHint;
  Element* mBackground;
  Element* mBorder;
};

//---------------------------------------------------------------- TextBoxButton
class TextBoxButton : public TextBox
{
public:
  /// Constructor.
  TextBoxButton(Composite* parent, StringParam iconName);

  /// Composite Interface.
  void UpdateTransform() override;

  IconButton* mButton;
};

//---------------------------------------------------------------- MultiLineText
class MultiLineText : public Composite
{
public:
  MultiLineText(Composite* parent, StringParam textClass = String());
  MultiLineText(Composite* parent, StringParam font, uint fontSize);

  void UpdateTransform() override;
  Vec2 GetMinSize() override;

  //Text on the label.
  void SetText(StringParam text);
  String GetText();

  float mMaxLineWidth;
  Text* mTextField;
  Element* mBackground;
  Element* mBorder;

private:
  void Initialize(Text* textObject);
};

namespace ModernTextBoxUi
{
  DeclareTweakable(Vec4, BackgroundColor);
  DeclareTweakable(Vec4, FocusBorderColor);
}

}//namespace Zero
