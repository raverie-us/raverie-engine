///////////////////////////////////////////////////////////////////////////////
///
/// \file TextEditorHotspot.cpp
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "TextEditorHotspot.hpp"
#include "TextEditor.hpp"

namespace Zero
{

TextEditorHotspot::TextEditorHotspot(StringRange regex)
  :mSearchRegex(regex,  RegexFlavor::EcmaScript, true)
{
}

// Only scan a line up to 2048 characters to avoid scanning
// very long lines of text
const uint MaxScanLength = 2048;

// Hyper link style
const uint HyperLinkStyle = 31;
// TODO DANE verify
void TextEditorHotspot::MarkHotspots(TextEditor* textEditor, int line)
{
  // Get current line information
  int lineLength = textEditor->GetLineLength(line);
  if(lineLength > MaxScanLength)
    return;

  char* buffer = (char*)alloca(lineLength+1);
  textEditor->GetLineText(line, buffer, lineLength+1);
  StringRange text(buffer);

  int lineStartPos = textEditor->GetPositionFromLine(line);

  // Check all attached hotspots
  forRange(TextEditorHotspot* hotspot, textEditor->mHotspots.All())
  {
    // Search the current line for the regex
    Matches matches;
    hotspot->mSearchRegex.Search(text, matches);
    if(matches.Size() > 0 )
    {
      int matchStart = text.FindFirstOf(matches.Front()).Begin() - text.Begin();
      int matchLength = matches.Front().ComputeRuneCount();
      textEditor->SetTextStyle(lineStartPos + matchStart, matchLength, HyperLinkStyle);
    }
  }
}

void TextEditorHotspot::MarkHotspots(TextEditor* textEditor)
{
  int lineCount = textEditor->GetLineCount();
  for(int line=0;line<lineCount;++line)
  {
    MarkHotspots(textEditor, line);
  }
}

void TextEditorHotspot::ClickHotspotsAt(TextEditor* textEditor, int position)
{
  // Find the line for this position
  int line = textEditor->GetLineFromPosition(position);
  int lineLength = textEditor->GetLineLength(line);

  if(lineLength > MaxScanLength)
    return;

  StringRange lineText = textEditor->GetLineText(line);
  int lineStart = textEditor->GetPositionFromLine(line);

  // Check all attached hotspots
  forRange(TextEditorHotspot* hotspot, textEditor->mHotspots.All())
  {
    // Re run the regex for capture groups
    Matches matches;
    hotspot->mSearchRegex.Search(lineText, matches);
    if(matches.Size() > 0 )
    {
      StringRange matchSub = lineText.FindFirstOf(matches.Front());
      int matchStartLine = matchSub.Begin() - lineText.Begin();
      int matchLength    = matchSub.End()   - lineText.Begin();
      int matchStart = matchStartLine + lineStart;
      int matchEnd   = matchStart     + matchLength;

      // Check the range
      if(position >= matchStart && position < matchEnd)
        hotspot->OnClick(matches);
    }
  }
}


}
