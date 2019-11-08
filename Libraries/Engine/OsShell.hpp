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

class OsShellHook
{
public:
  virtual void HookUpdate() = 0;
};

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

  /// Name of the Shell's operating system.
  String GetOsName();

  /// OS specific line-scroll setting when using the mouse scroll wheel.
  uint GetScrollLineCount();

  /// Get the rectangle of the primary monitor (desktop size).
  IntRect GetPrimaryMonitorRectangle();

  /// Get the size of the primary monitor (desktop size).
  IntVec2 GetPrimaryMonitorSize();

  /// Create an OS window.
  OsWindow* CreateOsWindow(StringParam windowName,
                           IntVec2Param clientSize,
                           IntVec2Param monitorClientPos,
                           OsWindow* parentWindow,
                           WindowStyleFlags::Enum flags,
                           WindowState::Enum state = WindowState::Windowed);

  /// Get the pixel color at the mouse position.
  ByteColor GetColorAtMouse();

  /// Set the cursor for the mouse.
  void SetMouseCursor(Cursor::Enum cursorId);

  /// Get an image of the desktop / primary monitor.
  bool GetPrimaryMonitorImage(Image* imageBuffer);

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

  /// Debug helper to print out memory logging information
  void DumpMemoryDebuggerStats();

  /// If this is set we will call the HookUpdate function inside here
  /// during the middle of our update after the keyboard has
  /// been updated but before we send input events.
  OsShellHook* mOsShellHook;

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

class SimpleSaveFileDialog : public SafeId32EventObject
{
public:
  typedef SimpleSaveFileDialog ZilchSelf;
  SimpleSaveFileDialog(StringParam data,
                       StringParam title,
                       StringParam filterName,
                       StringParam filter,
                       StringParam defaultExtension,
                       StringParam defaultFileName);

private:
  String mData;
  void OnFileDialogComplete(OsFileSelection* event);
};

} // namespace Zero
