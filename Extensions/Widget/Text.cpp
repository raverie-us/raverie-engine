///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const cstr cLocation = "EditorUi/Interaction";
Tweakable(bool, DebugTextArea, false, cLocation);

ZilchDefineType(Text, builder, type)
{
}

Text::Text(Composite* parent, StringParam style)
  :Widget(parent)
{
  BaseDefinition* textDefinition = parent->mDefSet->GetDefinition(style);
  ChangeDefinition(textDefinition);
  mSize = mFont->MeasureText(" ", 1.0f);
  mFontColor = Vec4(1);
  mAlign = TextAlign::Left;
  mMultiline = false;
  mClipText = true;
}

Text::Text(Composite* parent, StringParam fontName, uint fontSize)
  : Widget(parent)
{
  mFont = FontManager::GetInstance()->GetRenderFont(fontName, fontSize, 0);
  mSize = mFont->MeasureText(" ", 1.0f);
  mFontColor = Vec4(1);
  mAlign = TextAlign::Left;
  mMultiline = false;
  mClipText = true;
}

void Text::SizeToContents()
{
  mSize = GetMinSize();
}

void Text::ChangeDefinition(BaseDefinition* def)
{
  TextDefinition* textDefinition = (TextDefinition*)def; 
  mFont = textDefinition->mFont;
  mFontColor = textDefinition->FontColor;
}

void Text::SetMultiLine(bool multiLine)
{
  mMultiline = multiLine;
}

void Text::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  if (mText.SizeInBytes() == 0)
    return;

  Vec4 color = mFontColor * mColor * colorTx.ColorMultiply;

  ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, mFont->mTexture);
  FontProcessor fontProcessor(frameBlock.mRenderQueues, &viewNode, color);

  if (mMultiline)
    ProcessTextRange(fontProcessor, mFont, mText, Vec2::cZero, mAlign, Vec2(1, 1), mSize);
  else
    AddTextRange(fontProcessor, mFont, mText, Vec2::cZero, mAlign, Vec2(1, 1), mSize, mClipText);
}

Vec2 Text::GetBoundedSize(float maxWidth, float maxHeight)
{
  FontProcessorNoRender noRender;
  Vec2 limitedSize = Vec2(maxWidth, maxHeight);
  return ProcessTextRange(noRender, mFont, mText, Vec2(0, 0), mAlign, Vec2(1, 1), limitedSize);
}

void Text::FitToWidth(float maxWidth, float maxHeight)
{
  mSize = GetBoundedSize(maxWidth, maxHeight);
}

void Text::SetText(StringParam text)
{
  mText = text;
}

Vec2 Text::GetMinSize()
{
  if (mMultiline)
    return mSize;
  else if (mText.Empty())
    return mFont->MeasureText(" ");
  else
    return mFont->MeasureText(mText.All());
}

//---------------------------------------------------------------- Label Control
ZilchDefineType(Label, builder, type)
{
}

Label::Label(Composite* parent, StringParam style, StringParam text)
  : Composite(parent)
{
  FinishInitialize(style);
  SetText(text);
}

Label::~Label()
{

}

const Thickness DefaultLabelPadding = Thickness(2,2,2,2);

void Label::FinishInitialize(StringParam style)
{
  mDefSet = GetParent()->mDefSet;
  mText = new Text(this, style);
  mPadding = DefaultLabelPadding;
  mSize = ExpandSizeByThickness(mPadding, mText->GetMinSize());
}

void Label::SetText(StringParam text)
{
  mText->SetText(text);
}

void Label::SetTextClipping(bool value)
{
  mText->mClipText = value;
}

String Label::GetText()
{
  return mText->GetText();
}

RenderFont* Label::GetFont()
{
  return mText->mFont;
}

void Label::SizeToContents()
{
  mSize = GetMinSize();
}

Vec2 Label::GetMinSize()
{
  return ExpandSizeByThickness(mPadding, mText->GetMinSize());
}

void Label::UpdateTransform()
{
  Rect innerRect = RemoveThicknessRect(mPadding, mSize);
  PlaceWithRect(innerRect, mText);
  Composite::UpdateTransform();
}

}
