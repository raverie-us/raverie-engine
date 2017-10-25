///////////////////////////////////////////////////////////////////////////////
///
/// \file FontPattern.hpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

inline Vec2 Align(TextAlign::Enum textAlignement, Vec2& cur, float textOnLineSize, float lineSize, float scale)
{
  float extraSpaceOnLine = lineSize - textOnLineSize;
  switch (textAlignement)
  {
    case TextAlign::Right: return Vec2(cur.x + extraSpaceOnLine, cur.y);
    case TextAlign::Center: return Vec2(cur.x + Snap(extraSpaceOnLine * 0.5f, scale), cur.y);
    case TextAlign::Left: // default
    default: return cur;
  }
}

inline StringRange SkipRestOfLine(StringRange range)
{
  while(!range.Empty())
  {
    //Check for '\r'
    if(range.Front() == '\r' )
    {
      range.PopFront();
      //Check for new line '\r\n'
      if(!range.Empty() && range.Front() == '\n')
        range.PopFront();
      return range;
    }

    //Check for '\n'
    if(range.Front() == '\n')
    {
      range.PopFront();
      return range;
    }

    //Eat whitespace
    if(IsSpace(range.Front()))
      range.PopFront();
    else
      return range;
  }

  return range;
}

template <typename FontProcessor>
void AddTextRange(FontProcessor& processor, RenderFont* font, StringRange text, Vec2 textStart, TextAlign::Enum align, int line, Vec2 pixelScale, Vec2 textAreaSize, bool clipText = false)
{
  String displayText(text);
  if (clipText)
  {
    float sizeOfCharacters = font->MeasureText(text, 1.0f).x * pixelScale.x;
    if (textAreaSize.x >= sizeOfCharacters)
    {
      if (align == TextAlign::Right)
        textStart.x = Math::Round(textAreaSize.x - sizeOfCharacters);
      else if (align == TextAlign::Center)
        textStart.x = Math::Round(textAreaSize.x * 0.5f) - (sizeOfCharacters * 0.5f);
    }
    else
    {
      // Clip the text with the ellipses
      const StringRange chopText("...");
      float chopSize = font->MeasureText(chopText).x * pixelScale.x;

      while (!text.Empty() && sizeOfCharacters + chopSize > textAreaSize.x)
      {
        text.PopBack();
        sizeOfCharacters = font->MeasureText(text).x * pixelScale.x;
      }

      displayText = BuildString(text, chopText);
    }
  }

  Vec2 pos = textStart;

  forRange (Rune current, displayText.All())
  {
    RenderRune& r = font->GetRenderRune(current);

    if (current == NewLine)
      pos.y = font->mLineHeight * pixelScale.y;

    processor.ProcessRenderRune(r, pos, pixelScale, line);

    pos.x += r.Advance * pixelScale.x;
  }
}

template <typename FontProcessor>
Vec2 ProcessTextRange(FontProcessor& processor, RenderFont* font, StringRange text, Vec2 textStart, TextAlign::Enum textAlign, Vec2 pixelScale, Vec2 textAreaSize)
{
  Vec2 t = textStart;

  int line = 0;
  float lineSize = textAreaSize.x;
  float lineHeight = font->mLineHeight * pixelScale.y;
  float maxLineWidth = 0.0f;
  while(!text.Empty())
  {
    // Clip Text at bottom
    if (Math::Abs(t.y + lineHeight - textStart.y) > textAreaSize.y)
      break;

    uint charactersLeft = text.ComputeRuneCount();

    // Get number of characters till line end or new line.
    uint charactersTilLineEnd = font->GetPosition(text, lineSize, pixelScale.x, TextRounding::LastCharacter);

    if (charactersTilLineEnd >= charactersLeft)
    {
      // Line is longer than the number of character left
      float textSize = font->MeasureText(text, pixelScale.x).x;
      maxLineWidth = Math::Max(maxLineWidth, textSize);
      Vec2 textStart = Align(textAlign, t, textSize, lineSize, pixelScale.x);
      AddTextRange(processor, font, text, textStart, textAlign, line, pixelScale, textAreaSize);
      // No more text to render
      text = StringRange();
    }
    else
    {
      // Not enough room on line to draw all the text
      // clip the text to the line
      uint charactersToDraw = charactersTilLineEnd;
      StringIterator toDraw = text.Begin() + charactersToDraw;
      // Try to find a space to break the line
      while(toDraw.Data() > text.Data() && !IsSpace(toDraw.ReadCurrentRune()))
        --toDraw;

      // no white space found for split, just chop at last valid character
      if(toDraw.Data() == text.Data())
        toDraw = text.Begin() + charactersTilLineEnd;

      // Get spit text
      StringRange rangeToDraw = StringRange(text.Begin(), toDraw);

      float textSize = font->MeasureText(rangeToDraw, pixelScale.x).x;
      maxLineWidth = Math::Max(maxLineWidth, textSize);
      Vec2 textStart = Align(textAlign, t, textSize, lineSize, pixelScale.x);

      AddTextRange(processor, font, rangeToDraw, textStart, textAlign, line, pixelScale, textAreaSize);

      // build a range of what is left to draw
      text = StringRange(rangeToDraw.End(), text.End());

      // eat new line
      text = SkipRestOfLine(text);
    }

    //Move down a line and continue
    t.y += lineHeight;
    ++line;
  }

  // Return size used
  return Vec2(maxLineWidth, Math::Abs(t.y - textStart.y));
}

class FontProcessor
{
public:
  FontProcessor(RenderQueues* renderQueues, ViewNode* viewNode, Vec4 vertexColor);
  void ProcessRenderRune(RenderRune& rune, Vec2 position, Vec2 pixelScale, int line);

  RenderQueues* mRenderQueues;
  ViewNode* mViewNode;
  Vec4 mVertexColor;
};

class FontProcessorVertexArray
{
public:
  FontProcessorVertexArray(Vec4 vertexColor);
  void ProcessRenderRune(RenderRune& rune, Vec2 position, Vec2 pixelScale, int line);

  Array<StreamedVertex> mVertices;
  Vec4 mVertexColor;
};

class FontProcessorFindCharPosition
{
public:
  FontProcessorFindCharPosition(int charIndex, bool nextLine, Vec2 startPosition);
  void ProcessRenderRune(RenderRune& rune, Vec2 position, Vec2 pixelScale, int line);

  int mFindIndex;
  bool mNextLine;
  int mCurrentIndex;
  Vec2 mCharPosition;
  int mLastLine;
};

/// An index into a string that is rendered by SpriteText which may include word wrapping.
class CharacterIndex
{
public:
  ZilchDeclareType(TypeCopyMode::ValueType);
  CharacterIndex();
  CharacterIndex(int index);
  CharacterIndex(int index, int line, bool nextLine);

  /// The Rune index of a character (not the byte index).
  int mIndex;

  /// Which line this index originated from (starting from 0).
  int mLine;

  /// Whether the index is at the end of a line or the beginning of the next line.
  bool mNextLine;
};

class FontProcessorFindCharIndex
{
public:
  FontProcessorFindCharIndex(Vec2Param localPosition, float lineHeight);
  void ProcessRenderRune(RenderRune& rune, Vec2 position, Vec2 pixelScale, int line);

  Vec2 mLocalPosition;
  float mLineHeight;
  CharacterIndex mResult;
  float mClosestDistance;
  int mCurrentIndex;
  int mLastLine;
};

class FontProcessorNoRender
{
public:
  void ProcessRenderRune(RenderRune& rune, Vec2 position, Vec2 pixelScale, int line) {}
};

}
