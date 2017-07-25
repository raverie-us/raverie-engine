///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

struct ITaskbarList3;

namespace Zero
{

OsShell* CreateOsShellSystem();

DeclareEnum2(WindowsSystemUpdateState, Normal, SystemBlocking);

//-------------------------------------------------------------------WindowsOsWindow
/// Implementation of OsWindow for Windows
class WindowsOsWindow : public OsWindow
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  WindowsOsWindow();
  ~WindowsOsWindow();

  // OsWindow Interface
  IntVec2 GetPosition() override;
  void SetPosition(IntVec2Param screenPosition) override;

  IntVec2 GetSize() override;
  void SetSize(IntVec2Param size) override;

  void SetMinSize(IntVec2Param minSize) override;

  IntVec2 GetClientSize() override;
  void SetClientSize(IntVec2Param size) override;

  WindowsOsWindow* GetParent() override;

  IntVec2 ScreenToClient(IntVec2Param screenPoint) override;

  IntVec2 ClientToScreen(IntVec2Param clientPoint) override;

  WindowStyleFlags::Enum GetStyle() override;
  void SetStyle(WindowStyleFlags::Enum style) override;

  bool GetVisible() override;
  void SetVisible(bool visible) override;
  
  void SetTitle(StringParam title) override;

  WindowState::Enum GetState() override;
  void SetState(WindowState::Enum windowState) override;
  
  void TakeFocus() override;
  bool HasFocus() override;

  void Close() override;

  void Destroy() override;

  void ManipulateWindow(WindowBorderArea::Enum area) override;

  void SetMouseCapture(bool enabled) override;

  bool GetMouseTrap() override;
  void SetMouseTrap(bool mouseTrapped) override;
  
  void SetMouseCursor(Cursor::Enum cursorId) override;
  
  OsHandle GetWindowHandle() override;

  void SendProgress(ProgressType::Enum progressType, float progress) override;

  //Windows Procedure
  LRESULT WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

  friend class WindowsShellSystem;
  HWND mWindowHandle;
private:

  void SendKeyboardEvent(KeyboardEvent& keyEvent);
  void FillKeyboardEvent(Keys::Enum key, KeyState::Enum keyState, KeyboardEvent& keyEvent);
  void SendMouseButtonEvent(OsMouseEvent& mouseEvent, StringParam buttonState);
  void FillMouseEventData(IntVec2Param mousePosition, MouseButtons::Enum mouseButton, OsMouseEvent& mouseEvent);

  RECT GetDesktopClientRect();
  POINT GetMouseTrapScreenPosition();
  void CleanUp();

  WindowsShellSystem* mSystem;
  bool mIsMainWindow;
  bool mBorderless;
  IntVec2 mClientSize;
  IntVec2 mPosition;
  IntVec2 mMinSize;
  HCURSOR mCursor;
  bool mMouseCaptured;
  bool mMouseTrapped;
  Keyboard* mKeyboard;
  WindowStyleFlags::Enum mWindowStyle;
  WindowState::Enum mWindowState;
  WindowState::Enum mRestoreState;
  ITaskbarList3* mTaskBar;
  uint mTaskBarButtonCreated;
  WindowsOsWindow* mParent;
};

//-------------------------------------------------------------------WindowsShellSystem
/// Implementation of OsShell for Windows Platform
class WindowsShellSystem : public OsShell
{
public:
  WindowsShellSystem();
  ~WindowsShellSystem();

  // System Interface
  void Initialize(SystemInitializer& initializer) override;
  void Update() override;
  cstr GetName() override;

  // OsShell Interface
  String GetOsName() override;
  WindowsOsWindow* FindWindowAt(IntVec2Param position);
  PixelRect GetDesktopRect();
  WindowsOsWindow* CreateOsWindow(StringParam windowName, IntVec2Param windowSize, IntVec2Param windowPos,
                                  OsWindow* parentWindow, WindowStyleFlags::Enum flags) override;
  ByteColor GetColorAtMouse() override;
  bool IsClipboardText() override;
  String GetClipboardText() override;
  void SetClipboardText(StringParam text) override;
  bool IsClipboardImageAvailable() override;
  bool GetClipboardImage(Image* imageBuffer) override;
  bool GetWindowImage(Image* imageBuffer) override;
  void OpenFile(FileDialogConfig& config) override;
  void SaveFile(FileDialogConfig& config) override;
  void ShowMessageBox(StringParam title, StringParam message) override;
  size_t GetWindowCount() override;
  OsWindow* GetWindow(size_t index) override;

  friend class WindowsOsWindow;
private:

  Array<WindowsOsWindow*> mWindows;
  HWND mMainWindow;
  WindowsSystemUpdateState::Enum mState;
};

}//namespace Zero
