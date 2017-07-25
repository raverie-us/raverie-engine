///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class TextDefinition;

/// Text display object. Displays text at its position.
class Text : public Widget
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Text(Composite* parent, StringParam style);
  Text(Composite* parent, StringParam fontName, uint fontSize);

  // Basic text functions.
  void SetText(StringParam text);
  StringParam GetText(){return mText;}

  /// Returns the size used.
  void SetMultiLine(bool multiLine);

  void FitToWidth(float maxWidth, float maxHeight);

  Vec2 GetBoundedSize(float maxWidth, float maxHeight);

  // Widget Interface
  Vec2 GetMinSize() override;
  void SizeToContents() override;
  void ChangeDefinition(BaseDefinition* def) override;

  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override;

  RenderFont* mFont;
  Vec4 mFontColor;
  String mText;

  bool mClipText;
  TextAlign::Enum mAlign;
  bool mMultiline;
};

// Text with border area
class Label : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Label(Composite* parent, StringParam style = DefaultTextStyle, StringParam text = String());
  ~Label();

  RenderFont* GetFont();

  void SetText(StringParam text);
  String GetText();
  void SetTextClipping(bool value);

  // Widget Interface
  void SizeToContents() override;
  Vec2 GetMinSize() override;
  void UpdateTransform() override;

  Thickness mPadding;
  void FinishInitialize(StringParam style);
  Text* mText;
};


}
