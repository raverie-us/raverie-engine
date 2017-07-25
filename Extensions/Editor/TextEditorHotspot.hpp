///////////////////////////////////////////////////////////////////////////////
///
/// \file TextEditorHotspot.hpp
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
class TextEditor;
class MouseEvent;


/// Text Editor hotspot used to highlight text in a text editor
/// This uses a regular expression to match text on a line and
/// highlights with a hyperlink style hotpot. When clicked
/// it will call OnClick for the matched hotspot.
class TextEditorHotspot
{
public:
  Regex mSearchRegex;
  TextEditorHotspot(StringRange regex);

  /// Called when hotspot is matched. Matches
  /// are provides so capture groups can be used
  virtual void OnClick(Matches& matches)=0;

  /// Mark all hotspots found on specified line
  static void MarkHotspots(TextEditor* textEditor, int line);

  /// Mark all hotspots
  static void MarkHotspots(TextEditor* textEditor);

  /// Activate hotspots at location
  static void ClickHotspotsAt(TextEditor* textEditor, int position);
};

}
