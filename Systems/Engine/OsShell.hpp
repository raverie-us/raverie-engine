///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
namespace Events
{
// This event occurs in the middle of OsShell update before we process Os messages
DeclareEvent(OsShellUpdate);
}

class OsWindow;
class OsWindow;
struct FileDialogConfig;

class OsShellHook
{
public:
  virtual void HookUpdate() = 0;
};

//--------------------------------------------------------------------- OS Shell
/// Os Shell interface used to provide abstract platform user interface 
/// functionality. Used to manage mouse, keyboard, and clipboard functionality.
class OsShell : public System
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  OsShell();

  /// Name of the Shell's operating system.
  virtual String GetOsName() = 0;

  /// OS specific line-scroll setting when using the mouse scroll wheel.
  virtual uint GetScrollLineCount();

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

  /// If this is set we will call the HookUpdate function inside here
  /// during the middle of our update after the keyboard has
  /// been updated but before we send input events.
  OsShellHook* mOsShellHook;
};

//-------------------------------------------------------------------- Os Events
/// Files have been selected by the File Dialog.
class OsFileSelection : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  bool Success;
  Array<String> Files;
};

//-------------------------------------------------------------------FileDialogConfig
/// FileDialogConfig is used to configure the Open File Dialog
/// and the Save File Dialog.
struct FileDialogConfig : public FileDialogSetup
{
  FileDialogConfig();

  String EventName;
  Object* CallbackObject;

private:
  static void Callback(Array<String>& files, bool success, void* userData);
};

}//namespace Zero
