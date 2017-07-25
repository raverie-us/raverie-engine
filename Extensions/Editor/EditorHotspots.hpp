///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorHotspots.hpp
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
class MouseEvent;
class TextEditor;

// Simple Hyper links 
// Opens link on click
// Matches: http://zero.digipen.edu/
class HyperLinkHotspot : public TextEditorHotspot
{
public:
  HyperLinkHotspot();
  void OnClick(Matches& matches) override;
};

// Command Hotspot
// Runs command on click
// Matches: Command:DoSomething
class CommandHotspot : public TextEditorHotspot
{
public:
  CommandHotspot();
  void OnClick(Matches& matches) override;
};

// Resource Hotspot
// Edits resource on click
// Matches: 5423f6b5995af33f:ResourceName 
class ResourceHotspot : public TextEditorHotspot
{
public:
  ResourceHotspot();
  void OnClick(Matches& matches) override;
};

// Object Hotspot
// Select object on click
// Matches: <Cog 'Name' (Archetype) [142]>
class ObjectHotspot : public TextEditorHotspot
{
public:
  ObjectHotspot();
  void OnClick(Matches& matches) override;
};

// File Hotspot
// Opens file on click
// Matches: File "C:\File.z", line 33, message
class FileHotspot : public TextEditorHotspot
{
public:
  FileHotspot();
  void OnClick(Matches& matches) override;
};

}
