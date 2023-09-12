// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
namespace Events
{
// This event occurs in the middle of OsShell update before we process Os
// messages
DeclareEvent(Cut);
DeclareEvent(Copy);
DeclareEvent(Paste);
DeclareEvent(OsShellUpdate);
DeclareEvent(FileDialogComplete);
} // namespace Events

class OsWindow;
struct FileDialogConfig;

/// Os Shell interface used to provide abstract platform user interface
/// functionality. Used to manage mouse, keyboard, and clipboard functionality.
class OsShell : public System
{
public:
  ZilchDeclareType(OsShell, TypeCopyMode::ReferenceType);

  OsShell();

  /// System interface
  cstr GetName() override;
  void Update(bool debugger) override;

  /// OS specific line-scroll setting when using the mouse scroll wheel.
  uint GetScrollLineCount();

  /// Get the size of the primary monitor (desktop size).
  IntVec2 GetPrimaryMonitorSize();

  /// Get the pixel color at the mouse position.
  ByteColor GetColorAtMouse();

  /// Set the cursor for the mouse.
  void SetMouseCursor(Cursor::Enum cursorId);

  /// Use the file open dialog.
  void OpenFile(FileDialogConfig* config);
  /// Use the save file dialog.
  void SaveFile(FileDialogConfig* config);
  /// Message box used for critical failures.
  void ShowMessageBox(StringParam title, StringParam message);

  /// Scan for new input devices and register them with Zero.
  void ScanInputDevices();

  /// How many OsWindows current exist
  size_t GetWindowCount();
  OsWindow* GetWindow(size_t index);

  // Internal

  /// Platform specific shell
  Shell mShell;

  static void ShellOnCopy(ClipboardData& data, bool cut, Shell* shell);
  static void ShellOnPaste(const ClipboardData& data, Shell* shell);
};

class ClipboardEvent : public Event, public ClipboardData
{
public:
  ZilchDeclareType(ClipboardEvent, TypeCopyMode::ReferenceType);
  void Clear();
  void SetText(StringParam text);
  String GetText();
  void SetImage(const Image& image);
  const Image& GetImage();
  bool mHandled = false;
};

/// Files have been selected by the File Dialog.
class OsFileSelection : public Event
{
public:
  ZilchDeclareType(OsFileSelection, TypeCopyMode::ReferenceType);
  bool Success;
  Array<String> Files;
};

/// FileDialogConfig is used to configure the Open File Dialog
/// and the Save File Dialog.
/// Note that the config may only be used ONCE because it will be automatically
/// deleted at the end of the OpenFile/SaveFile call.
struct FileDialogConfig : public FileDialogInfo
{
  // The default event name is Events::FileDialogComplete.
  String EventName;
  HandleOf<Object> CallbackObject;

  static FileDialogConfig* Create();

private:
  FileDialogConfig();
  static void Callback(Array<String>& files, void* userData);
};

} // namespace Zero
