///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
namespace Z
{
// This is only used in extreme cases where we integrate an API that also attempts to make OS calls (eg Chrome)
extern bool gEnableOsWindowProcedure;
}

class Image;
class OsWindow;
class OsWindow;
struct FileDialogConfig;

//--------------------------------------------------------------------- OS Shell
/// Os Shell interface used to provide abstract platform user interface 
/// functionality. Used to manage mouse, keyboard, and clipboard functionality.
class OsShell : public System
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Name of the Shell's operating system.
  virtual String GetOsName() = 0;

  /// Find what OsWindow is underneath the given screen position.
  virtual OsWindow* FindWindowAt(IntVec2Param screenPosition) = 0;

  // Get the desktop rectangle.
  virtual PixelRect GetDesktopRect() = 0;

  /// Create an OS window.
  virtual OsWindow* CreateOsWindow(StringParam windowName, IntVec2Param windowSize, IntVec2Param windowPos,
                                   OsWindow* parentWindow, WindowStyleFlags::Enum flags) = 0;

  /// Get the pixel color at the mouse position.
  virtual ByteColor GetColorAtMouse() = 0;
  
  /// Check if the current clipboard Contains text.
  virtual bool IsClipboardText() = 0;
  /// The current clipboard text.
  virtual String GetClipboardText() = 0;
  virtual void SetClipboardText(StringParam text) = 0;
  
  /// Checks if the clipboard holds an image
  virtual bool IsClipboardImageAvailable() = 0;
  /// Get an image from clipboard.
  virtual bool GetClipboardImage(Image* imageBuffer) = 0;
  /// Get an image from window
  virtual bool GetWindowImage(Image* imageBuffer) = 0;

  // Use the file open dialog.
  virtual void OpenFile(FileDialogConfig& config) = 0;
  // Use the save file dialog.
  virtual void SaveFile(FileDialogConfig& config) = 0;
  // Message box used for critical failures.
  virtual void ShowMessageBox(StringParam title, StringParam message) = 0;

  /// How many OsWindows current exist
  virtual size_t GetWindowCount() = 0;
  virtual OsWindow* GetWindow(size_t index) = 0;
  
  /// Debug helper to print out memory logging information
  void DumpMemoryDebuggerStats();
};

//-------------------------------------------------------------------- Os Events
/// Files have been selected by the File Dialog.
class OsFileSelection : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  bool Success;
  String EventName;
  Array<String> Files;
};

//-----------------------------------------------------------FileDialogFilter
DeclareBitField2(FileDialogFlags, MultiSelect, Folder);

struct FileDialogFilter
{
  FileDialogFilter();
  FileDialogFilter(StringParam filter);
  FileDialogFilter(StringParam description, StringParam filter);

  String mDescription;
  // i.e. "*.fbx"
  String mFilter;
};

//-------------------------------------------------------------------FileDialogConfig
/// FileDialogConfig is used to configure the Open File Dialog
/// and the Save File Dialog.
struct FileDialogConfig
{
  FileDialogConfig();
  void AddFilter(StringParam description, StringParam filter);

  String Title;
  String EventName;
  String StartingDirectory;
  Array<FileDialogFilter> mSearchFilters;
  // Should not include '.'
  String mDefaultSaveExtension;
  String DefaultFileName;
  FileDialogFlags::Type Flags;
  Object* CallbackObject;
};

}//namespace Zero
