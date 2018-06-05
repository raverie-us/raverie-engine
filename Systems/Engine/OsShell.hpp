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
  ZilchDeclareType(OsShell, TypeCopyMode::ReferenceType);

  OsShell();

  /// System interface
  cstr GetName() override;
  void Update() override;

  /// Name of the Shell's operating system.
  String GetOsName();

  /// OS specific line-scroll setting when using the mouse scroll wheel.
  uint GetScrollLineCount();

  /// Get the rectangle of the primary monitor (desktop size).
  IntRect GetPrimaryMonitorRectangle();

  /// Get the size of the primary monitor (desktop size).
  IntVec2 GetPrimaryMonitorSize();

  /// Create an OS window.
  OsWindow* CreateOsWindow(
    StringParam windowName,
    IntVec2Param clientSize,
    IntVec2Param monitorClientPos,
    OsWindow* parentWindow,
    WindowStyleFlags::Enum flags);

  /// Get the pixel color at the mouse position.
  ByteColor GetColorAtMouse();

  /// Set the cursor for the mouse.
  void SetMouseCursor(Cursor::Enum cursorId);
  
  /// Check if the current clipboard Contains text.
  bool IsClipboardText();
  /// The current clipboard text.
  String GetClipboardText();
  void SetClipboardText(StringParam text);
  
  /// Checks if the clipboard holds an image
  bool IsClipboardImage();
  /// Get an image from clipboard.
  bool GetClipboardImage(Image* imageBuffer);
  /// Get an image of the desktop / primary monitor.
  bool GetPrimaryMonitorImage(Image* imageBuffer);

  /// Use the file open dialog.
  bool OpenFile(FileDialogConfig& config);
  /// Use the save file dialog.
  bool SaveFile(FileDialogConfig& config);
  /// Message box used for critical failures.
  void ShowMessageBox(StringParam title, StringParam message);

  /// Scan for new input devices and register them with Zero.
  void ScanInputDevices();

  /// How many OsWindows current exist
  size_t GetWindowCount();
  OsWindow* GetWindow(size_t index);
  
  /// Debug helper to print out memory logging information
  void DumpMemoryDebuggerStats();

  /// If this is set we will call the HookUpdate function inside here
  /// during the middle of our update after the keyboard has
  /// been updated but before we send input events.
  OsShellHook* mOsShellHook;

  bool mIsUpdating;

  /// Platform specific shell
  Shell mShell;
};

//-------------------------------------------------------------------- Os Events
/// Files have been selected by the File Dialog.
class OsFileSelection : public Event
{
public:
  ZilchDeclareType(OsFileSelection, TypeCopyMode::ReferenceType);
  bool Success;
  Array<String> Files;
};

//-------------------------------------------------------------------FileDialogConfig
/// FileDialogConfig is used to configure the Open File Dialog
/// and the Save File Dialog.
struct FileDialogConfig : public FileDialogInfo
{
  FileDialogConfig();

  String EventName;
  Object* CallbackObject;

private:
  static void Callback(Array<String>& files, void* userData);
};

}//namespace Zero
