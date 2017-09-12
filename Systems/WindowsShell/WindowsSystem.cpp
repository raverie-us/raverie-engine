///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

// These values must be defined to handle the WM_MOUSEHWHEEL on 
// Windows 2000 and Windows XP, the first two values will be defined 
// in the Longhorn SDK and the last value is a default value 
// that will not be defined in the Longhorn SDK but will be needed for 
// handling WM_MOUSEHWHEEL messages emulated by IntelliType Pro
// or IntelliPoint (if implemented in future versions).
#define WM_MOUSEHWHEEL 0x020E

namespace Zero
{ 

void DestroyRenderer();

OsShell* CreateOsShellSystem()
{
  return new WindowsShellSystem();
}

#define ProcessInput(wintype, key) case wintype: return key;

Keys::Enum TranslateKeyCode(WPARAM wParam)
{
  if((wParam >= 'A' && wParam <= 'Z') || 
     (wParam >= '0' && wParam <= '9'))
  {
    return (Keys::Enum)wParam;
  }
  else
  {
    switch(wParam)
    {
#include "WinInputs.inl"
    }
  }
  return Keys::Unknown;
}

bool IsKeyDown(uint keyCode)
{
  return GetKeyState(keyCode) < 0;
}

bool IsShiftHeld()
{
  return IsKeyDown(VK_LSHIFT) || IsKeyDown(VK_RSHIFT);
}

bool IsCtrlHeld()
{
  return IsKeyDown(VK_LCONTROL) || IsKeyDown(VK_RCONTROL);
}

bool IsAltHeld()
{
  return IsKeyDown(VK_LMENU) || IsKeyDown(VK_RMENU);
}

bool IsSpaceHeld()
{
  return IsKeyDown(VK_SPACE);
}

enum WindowArea
{
  SC_SIZE_HTLEFT = 1,
  SC_SIZE_HTRIGHT = 2,
  SC_SIZE_HTTOP = 3,
  SC_SIZE_HTTOPLEFT = 4,
  SC_SIZE_HTTOPRIGHT = 5,
  SC_SIZE_HTBOTTOM = 6,
  SC_SIZE_HTBOTTOMLEFT = 7,
  SC_SIZE_HTBOTTOMRIGHT = 8
};

uint RectWidth(RECT& rect)
{
  return rect.right - rect.left;
}

uint RectHeight(RECT& rect)
{
  return rect.bottom - rect.top;
}

PixelRect FromRECT(RECT& rect)
{
  PixelRect p;
  p.X = rect.left;
  p.Y = rect.top;
  p.SizeX = RectWidth(rect);
  p.SizeY = RectHeight(rect);
  return p;
}

IntVec2 ToIntVec2(POINT& point)
{
  return IntVec2(point.x, point.y);
}

IntVec2 LocalClientToScreen(HWND window, IntVec2 point)
{
  IntVec2 newPoint = point;
  ::ClientToScreen(window, (POINT*)&newPoint);
  return newPoint;
}

IntVec2 LocalScreenToClient(HWND window, IntVec2 point)
{
  IntVec2 newPoint = point;
  ::ScreenToClient(window, (POINT*)&newPoint);
  return newPoint;
}

DWORD Win32StyleFromWindowStyle(WindowStyleFlags::Enum styleFlags)
{
  DWORD win32Style = WS_POPUP | WS_VISIBLE;
  if(styleFlags & WindowStyleFlags::NotVisible)
    win32Style &= ~WS_VISIBLE;
  if(styleFlags & WindowStyleFlags::TitleBar)
    win32Style |= WS_CAPTION;
  if(styleFlags & WindowStyleFlags::Resizable)
    win32Style |= WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
  if(styleFlags & WindowStyleFlags::Close)
    win32Style |= WS_SYSMENU;
  // Maximize size not correct with caption on and borderless
  if(styleFlags & WindowStyleFlags::ClientOnly)
    win32Style &= ~WS_CAPTION;
  return win32Style;
}

DWORD GetWin32ExStyle(WindowStyleFlags::Enum styleFlags)
{
  if(styleFlags & WindowStyleFlags::OnTaskBar)
    return WS_EX_APPWINDOW;
  else
    return WS_EX_TOOLWINDOW;
}

//-------------------------------------------------------------------WindowsOsWindow
ZilchDefineType(WindowsOsWindow, builder, type)
{
  ZilchBindConstructor();
  ZilchBindDestructor();
}

WindowsOsWindow::WindowsOsWindow()
{
  mTaskBar = nullptr;
  mTaskBarButtonCreated = 0;
  mIsMainWindow = false;
  mWindowHandle = nullptr;
  mMouseTrapped = false;
  mMouseCaptured = false;
  mCursor = nullptr;
  mSystem = nullptr;
  mParent = nullptr;
  mClientSize = IntVec2::cZero;
  mPosition = IntVec2::cZero;
  mKeyboard = Keyboard::GetInstance();
  mMinSize = IntVec2(10, 10);
  mWindowStyle = (WindowStyleFlags::Enum)WindowStyleFlags::None;
  mBorderless = false;
}

WindowsOsWindow::~WindowsOsWindow()
{
  CleanUp();
}

IntVec2 WindowsOsWindow::GetPosition()
{
  RECT clientRect;
  GetWindowRect(mWindowHandle, &clientRect);
  return IntVec2(clientRect.left, clientRect.top);
}

void WindowsOsWindow::SetPosition(IntVec2Param newPosition)
{
  mPosition = newPosition;
  SetWindowPos(mWindowHandle, nullptr, newPosition.x, newPosition.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

IntVec2 WindowsOsWindow::GetSize()
{
  RECT rect = GetDesktopClientRect();
  return IntVec2(rect.right - rect.left, rect.bottom - rect.top);
}

void WindowsOsWindow::SetSize(IntVec2Param size)
{
  SetWindowPos(mWindowHandle, 0, 0, 0, size.x, size.y, 
    SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOMOVE);
}

void WindowsOsWindow::SetMinSize(IntVec2Param minSize)
{
  mMinSize = minSize;
}

IntVec2 WindowsOsWindow::GetClientSize()
{
  RECT rect = GetDesktopClientRect();
  return IntVec2(rect.right - rect.left, rect.bottom - rect.top);
}

void WindowsOsWindow::SetClientSize(IntVec2Param size)
{
  RECT rect = { 0, 0, size.x, size.y };
  if (!mBorderless)
    AdjustWindowRect(&rect, Win32StyleFromWindowStyle(mWindowStyle), FALSE);

  SetWindowPos(mWindowHandle, 0, 0, 0, RectWidth(rect), RectHeight(rect), 
    SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOMOVE);
}

WindowsOsWindow* WindowsOsWindow::GetParent()
{
  return mParent;
}

IntVec2 WindowsOsWindow::ScreenToClient(IntVec2Param screenPoint)
{
  return LocalScreenToClient(mWindowHandle, screenPoint);
}

IntVec2 WindowsOsWindow::ClientToScreen(IntVec2Param clientPoint)
{
  return LocalClientToScreen(mWindowHandle, clientPoint);
}

WindowStyleFlags::Enum WindowsOsWindow::GetStyle()
{
  return mWindowStyle;
}

void WindowsOsWindow::SetStyle(WindowStyleFlags::Enum windowStyle)
{
  mWindowStyle = windowStyle;
  mBorderless = (windowStyle & WindowStyleFlags::ClientOnly);
  DWORD win32 = Win32StyleFromWindowStyle(windowStyle);
  SendMessage(mWindowHandle, WM_SYSCOMMAND, SC_RESTORE, 0);
  SetWindowLong(mWindowHandle, GWL_STYLE, win32);
}

bool WindowsOsWindow::GetVisible()
{
  return true;
}

void WindowsOsWindow::SetVisible(bool visible)
{
  ShowWindow(mWindowHandle, visible ? SW_SHOW : SW_HIDE);
}

void WindowsOsWindow::SetTitle(StringParam title)
{
  SetWindowText(mWindowHandle, Widen(title).c_str());
}

WindowState::Enum WindowsOsWindow::GetState()
{
  WINDOWPLACEMENT placement = { sizeof(placement) };
  GetWindowPlacement(mWindowHandle, &placement);
  if (placement.showCmd == SW_SHOWNORMAL)
  {
    HMONITOR monitor = MonitorFromWindow(mWindowHandle, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo = { sizeof(monitorInfo) };
    GetMonitorInfo(monitor, &monitorInfo);
    RECT monitorRect = monitorInfo.rcMonitor;

    RECT rect = placement.rcNormalPosition;

    // Fullscreen is done in windowed mode, check if window size is the same as the monitor.
    if (rect.left == monitorRect.left && rect.top == monitorRect.top && rect.right == monitorRect.right && rect.bottom == monitorRect.bottom)
      return WindowState::Fullscreen;

    return WindowState::Windowed;
  }
  else if (placement.showCmd == SW_SHOWMINIMIZED)
    return WindowState::Minimized;
  else if (placement.showCmd == SW_SHOWMAXIMIZED)
    return WindowState::Maximized;
  else
    return WindowState::Windowed;
}

void WindowsOsWindow::SetState(WindowState::Enum windowState)
{
  static WINDOWPLACEMENT sPlacement;
  static WindowStyleFlags::Enum sRestorStyle;

  switch (windowState)
  {
    case WindowState::Minimized:
    {
      // Intel crashes if minimizing from our fullscreen state, so revert to windowed first.
      if (Z::gEngine->mIntel && GetState() == WindowState::Fullscreen)
      {
        SetStyle(sRestorStyle);
        SetWindowPlacement(mWindowHandle, &sPlacement);
      }

      SendMessage(mWindowHandle, WM_SYSCOMMAND, SC_MINIMIZE, 0);
      break;
    }
    case WindowState::Windowed:
    {
      // Switching back to windowed mode is the only way to leave fullscreen.
      // This is the only location that style and placement should need to be reset (except intel bug above).
      if (GetState() == WindowState::Fullscreen)
      {
        SetStyle(sRestorStyle);
        SetWindowPlacement(mWindowHandle, &sPlacement);
      }

      SendMessage(mWindowHandle, WM_SYSCOMMAND, SC_RESTORE, 0);
      // Force window to update
      SetWindowPos(mWindowHandle, nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
      break;
    }
    case WindowState::Maximized:
    {
      SendMessage(mWindowHandle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
      break;
    }
    case WindowState::Fullscreen:
    {
      // Make sure in windowed mode to save placement.
      SendMessage(mWindowHandle, WM_SYSCOMMAND, SC_RESTORE, 0);
      GetWindowPlacement(mWindowHandle, &sPlacement);

      // Remove border and disable window's aero so it can't manipulate the window without us knowing.
      sRestorStyle = mWindowStyle;
      mWindowStyle = (WindowStyleFlags::Enum)(mWindowStyle | WindowStyleFlags::ClientOnly);
      mWindowStyle = (WindowStyleFlags::Enum)(mWindowStyle & ~WindowStyleFlags::Resizable);
      SetStyle(mWindowStyle);

      HMONITOR monitor = MonitorFromWindow(mWindowHandle, MONITOR_DEFAULTTONEAREST);
      MONITORINFO monitorInfo = { sizeof(monitorInfo) };
      GetMonitorInfo(monitor, &monitorInfo);
      RECT rect = monitorInfo.rcMonitor;

      SetPosition(IntVec2(rect.left, rect.top));
      SetClientSize(IntVec2(rect.right - rect.left, rect.bottom - rect.top));

      break;
    }
    case WindowState::Restore:
    {
      SendMessage(mWindowHandle, WM_SYSCOMMAND, SC_RESTORE, 0);
      break;
    }
  }
}

IntVec2 WindowsOsWindow::GetPrimaryScreenSize()
{
  return IntVec2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
}

void WindowsOsWindow::TakeFocus()
{
  //JoshD: There's a lot of extra stuff required to make a window's window take
  // focus (required for the launcher). Talk to me if you need to change this.

  // Force the window to be un-minimized
  SetState(WindowState::Restore);
  
  // Sometimes windows won't actually take focus...just make it happen...hopefully...
  for(size_t i = 0; i < 5; ++i)
  {
    SetForegroundWindow(mWindowHandle);
    SetFocus(mWindowHandle);
    BringWindowToTop(mWindowHandle);
    SetActiveWindow(mWindowHandle);
  }
}

bool WindowsOsWindow::HasFocus()
{
  return GetActiveWindow() == mWindowHandle;
}

void WindowsOsWindow::Close()
{
  SendMessage(mWindowHandle, WM_SYSCOMMAND, SC_CLOSE, 0);
}

void WindowsOsWindow::Destroy()
{
  CleanUp();
}

void WindowsOsWindow::ManipulateWindow(WindowBorderArea::Enum area)
{
  WPARAM param = 0;
  switch(area)
  {
    case WindowBorderArea::Title:       param = SC_MOVE | HTCAPTION; break;
    case WindowBorderArea::TopLeft :    param = SC_SIZE | SC_SIZE_HTTOPLEFT; break;
    case WindowBorderArea::Top:         param = SC_SIZE | SC_SIZE_HTTOP; break;
    case WindowBorderArea::TopRight:    param = SC_SIZE | SC_SIZE_HTTOPRIGHT; break;
    case WindowBorderArea::Left:        param = SC_SIZE | SC_SIZE_HTLEFT; break;
    case WindowBorderArea::Right:       param = SC_SIZE | SC_SIZE_HTRIGHT; break;
    case WindowBorderArea::BottomLeft:  param = SC_SIZE | SC_SIZE_HTBOTTOMLEFT; break;
    case WindowBorderArea::Bottom:      param = SC_SIZE | SC_SIZE_HTBOTTOM; break;
    case WindowBorderArea::BottomRight: param = SC_SIZE | SC_SIZE_HTBOTTOMRIGHT; break;
  }

  SendMessage(mWindowHandle, WM_SYSCOMMAND, param, 0);

  // The window procedure never gets a mouse up after dragging
  // so send one now for double click to work
  POINT cursorPos;
  GetCursorPos(&cursorPos);
  IntVec2 clientPoint = this->ScreenToClient(ToIntVec2(cursorPos));

  OsMouseEvent mouseEvent;
  FillMouseEventData(clientPoint, MouseButtons::Left, mouseEvent);
  SendMouseEvent(mouseEvent, Events::OsMouseUp);
}

void WindowsOsWindow::SetMouseCapture(bool capture)
{
  if(capture)
    ::SetCapture(mWindowHandle);
  else
    ::ReleaseCapture();

  mMouseCaptured = capture;
}

bool WindowsOsWindow::GetMouseTrap()
{
  return mMouseTrapped;
}

void WindowsOsWindow::SetMouseTrap(bool mouseTrapped)
{
  mMouseTrapped = mouseTrapped;
  if(mouseTrapped)
  {
    // Clip the cursor to the client area
    RECT clientRect = this->GetDesktopClientRect();
    ::ClipCursor(&clientRect);
    SetMouseCursor(Cursor::Invisible);
  }
  else
  {
    // Remove mouse clipping
    ::ClipCursor(nullptr);
    SetMouseCursor(Cursor::Arrow);
  }
}

void WindowsOsWindow::SetMouseCursor(Cursor::Enum cursorId)
{
  if(cursorId == Cursor::Invisible)
  {
    mCursor = nullptr;
  }
  else
  {
    LPWSTR cursorName = 0;
    switch(cursorId)
    {
      case Cursor::Arrow:    cursorName = IDC_ARROW;     break;
      case Cursor::Wait:     cursorName = IDC_WAIT;      break;
      case Cursor::Cross:    cursorName = IDC_CROSS;     break;
      case Cursor::SizeNWSE: cursorName = IDC_SIZENWSE;  break;
      case Cursor::SizeNESW: cursorName = IDC_SIZENESW;  break;
      case Cursor::SizeWE:   cursorName = IDC_SIZEWE;    break;
      case Cursor::SizeNS:   cursorName = IDC_SIZENS;    break;
      case Cursor::SizeAll:  cursorName = IDC_SIZEALL;   break;
      case Cursor::TextBeam: cursorName = IDC_IBEAM;     break;
      case Cursor::Hand:     cursorName = IDC_HAND;      break;
    }
    mCursor = LoadCursor(nullptr, cursorName);
  }

  SetCursor(mCursor);
}

OsHandle WindowsOsWindow::GetWindowHandle()
{
  return (OsHandle)mWindowHandle;
}

void WindowsOsWindow::SendProgress(ProgressType::Enum progressType, float progress)
{
  // If the task bar button has been created
  // Update the progress bar built into the task bar.
  if(mTaskBar)
  {
    if(progressType == ProgressType::None)
    {
      // Disable progress
      mTaskBar->SetProgressState(mWindowHandle, TBPF_NOPROGRESS);
    }
    else if(progressType == ProgressType::Indeterminate)
    {
      // indeterminate progress (spinning animation)
      mTaskBar->SetProgressState(mWindowHandle, TBPF_INDETERMINATE);
    }
    else
    {
      // actual progress
      mTaskBar->SetProgressState(mWindowHandle, TBPF_NORMAL);

      // Progress is a normalized float change it to a ULONGLONG for windows.
      const uint ProgressValueScale = 10000000;
      mTaskBar->SetProgressValue(mWindowHandle, (ULONGLONG)(progress * ProgressValueScale), 
        (ULONGLONG)ProgressValueScale);
    }
  }
}

void WindowsOsWindow::SendKeyboardEvent(KeyboardEvent& event, bool simulated)
{
  if (mOsInputHook)
    mOsInputHook->HookKeyboardEvent(event);

  if (mBlockUserInput && !simulated)
    return;

  mKeyboard->UpdateKeys(event);

  DispatchEvent(cOsKeyboardEventsFromState[event.State], &event);

  mKeyboard->DispatchEvent(cKeyboardEventsFromState[event.State], &event);
}

void WindowsOsWindow::SendKeyboardTextEvent(KeyboardTextEvent& event, bool simulated)
{
  if (mOsInputHook)
    mOsInputHook->HookKeyboardTextEvent(event);

  if (mBlockUserInput && !simulated)
    return;

  DispatchEvent(event.EventId, &event);
  Keyboard::GetInstance()->DispatchEvent(event.EventId, &event);
}

void WindowsOsWindow::SendMouseEvent(OsMouseEvent& event, bool simulated)
{
  if (mOsInputHook)
    mOsInputHook->HookMouseEvent(event);

  if (mBlockUserInput && !simulated)
    return;

  DispatchEvent(event.EventId, &event);
}

void WindowsOsWindow::SendMouseDropEvent(OsMouseDropEvent& event, bool simulated)
{
  if (mOsInputHook)
    mOsInputHook->HookMouseDropEvent(event);

  if (mBlockUserInput && !simulated)
    return;

  DispatchEvent(event.EventId, &event);
}

void WindowsOsWindow::SendWindowEvent(OsWindowEvent& event, bool simulated)
{
  if (mOsInputHook)
    mOsInputHook->HookWindowEvent(event);

  if (mBlockUserInput && !simulated)
    return;

  DispatchEvent(event.EventId, &event);
}

void WindowsOsWindow::FillKeyboardEvent(Keys::Enum key, KeyState::Enum keyState, KeyboardEvent& keyEvent)
{
  keyEvent.Key = key;
  keyEvent.State = keyState;
  keyEvent.mKeyboard = mKeyboard;
  keyEvent.AltPressed = IsAltHeld();
  keyEvent.CtrlPressed = IsCtrlHeld();
  keyEvent.ShiftPressed = IsShiftHeld();
  keyEvent.SpacePressed = IsSpaceHeld();
}

void WindowsOsWindow::SendMouseEvent(OsMouseEvent& mouseEvent, StringParam buttonState)
{
  mouseEvent.EventId = buttonState;
  SendMouseEvent(mouseEvent, false);
}

void WindowsOsWindow::FillMouseEventData(IntVec2Param mousePosition, MouseButtons::Enum mouseButton, OsMouseEvent& mouseEvent)
{ 
  mouseEvent.Window = this;
  mouseEvent.ClientPosition = mousePosition;
  mouseEvent.MouseButton = MouseButtons::None;
  mouseEvent.ScrollMovement = Vec2(0,0);
  mouseEvent.AltPressed = IsAltHeld();
  mouseEvent.CtrlPressed = IsCtrlHeld();
  mouseEvent.ShiftPressed = IsShiftHeld();
  mouseEvent.ButtonDown[MouseButtons::Left] = IsKeyDown(VK_LBUTTON);
  mouseEvent.ButtonDown[MouseButtons::Right] = IsKeyDown(VK_RBUTTON);
  mouseEvent.ButtonDown[MouseButtons::Middle] = IsKeyDown(VK_MBUTTON);
  mouseEvent.ButtonDown[MouseButtons::XOneBack] = IsKeyDown(VK_XBUTTON1);
  mouseEvent.ButtonDown[MouseButtons::XTwoForward] = IsKeyDown(VK_XBUTTON2);
  // Set the none button to false
  mouseEvent.ButtonDown[MouseButtons::None] = 0;
  mouseEvent.MouseButton = mouseButton;
}

RECT WindowsOsWindow::GetDesktopClientRect()
{
  RECT clientRect;
  GetClientRect(mWindowHandle, &clientRect);
  ::ClientToScreen(mWindowHandle, (LPPOINT) &clientRect.left);
  ::ClientToScreen(mWindowHandle, (LPPOINT) &clientRect.right);
  return clientRect;
}

POINT WindowsOsWindow::GetMouseTrapScreenPosition()
{
  // Trap the mouse in the center of the window
  RECT clientRect = this->GetDesktopClientRect();
  int sizeX = clientRect.right - clientRect.left;
  int sizeY = clientRect.bottom - clientRect.top;

  POINT result;
  result.x = clientRect.left + sizeX / 2;
  result.y = clientRect.top + sizeY / 2;
  return result;
}

void WindowsOsWindow::CleanUp()
{
  // Remove pointer to window
  SetWindowPointer(mWindowHandle, nullptr);

  DestroyWindow(mWindowHandle);

  mWindowHandle = nullptr;
}

cwstr cWindowClassName = L"ZeroWindow";

LRESULT CALLBACK StaticWindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (msg == WM_CREATE)
  {
    // Window is being created get the WindowsOsWindow from the lParam
    // this is passed in from CreateWindow
    WindowsOsWindow* osWindow = (WindowsOsWindow*)((CREATESTRUCT*)(lParam))->lpCreateParams;

    // Set it on the user data section of the window
    SetWindowPointer(hwnd, osWindow);

    osWindow->mWindowHandle = hwnd;
  }

  // Get the window associated with this hwnd from the user data section
  WindowsOsWindow* osWindow = (WindowsOsWindow*)PointerFromWindow(hwnd);
  if(osWindow != nullptr)
  {
    return osWindow->WindowProcedure(hwnd, msg, wParam, lParam);
  }
  else
  {
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
}

bool RegisterWindowClass(HINSTANCE hInstance)
{
  WNDCLASSEX winClass;
  ZeroMemory(&winClass, sizeof(WNDCLASSEX));

  winClass.lpszClassName = cWindowClassName;
  winClass.cbSize        = sizeof(WNDCLASSEX);
  winClass.style         = CS_OWNDC;
  winClass.lpfnWndProc   = StaticWindowProcedure;
  winClass.hInstance     = hInstance;
  winClass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(ZeroDefaultIcon));
  winClass.hIconSm       = LoadIcon(hInstance, MAKEINTRESOURCE(ZeroDefaultIcon));
  winClass.hCursor       = nullptr;
  winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  winClass.lpszMenuName  = nullptr;
  winClass.cbClsExtra    = sizeof(void*);
  winClass.cbWndExtra    = sizeof(void*);

  if(!RegisterClassEx(&winClass))
    return false;

  return true;
}

void UnregisterWindowClass(HINSTANCE hInstance)
{
  UnregisterClass(cWindowClassName, hInstance);
}

IntVec2 PositionFromLParam(LPARAM lParam)
{
  // Systems with multiple monitors can have negative x and y coordinates
  return IntVec2((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
}

MouseButtons::Enum ButtonFromWParam(WPARAM wParam)
{
  if(HIWORD(wParam) == XBUTTON1)
    return MouseButtons::XOneBack;
  else
    return MouseButtons::XTwoForward;
}

const uint MessageHandled = 0;

LRESULT WindowsOsWindow::WindowProcedure(HWND hwnd, UINT messageId, WPARAM wParam, LPARAM lParam)
{
  switch(messageId)
  {
    case WM_CLOSE:
    {
      OsWindowEvent event;
      this->DispatchEvent(Events::OsClose, &event);
      return MessageHandled;
    }

    // Sent when a window is being destroyed.
    case WM_DESTROY:
    {
      return MessageHandled;
    }

    case WM_SYSCHAR:
    {
      //this will stop the beep when pressing alt
      return MessageHandled;
    }

    // Window has been activated
    case WM_ACTIVATE:
    {
      DWORD activated = LOWORD(wParam);

      OsWindowEvent focusEvent;
      if (activated)
      {
        focusEvent.EventId = Events::OsFocusGained;
      }
      else
      {
        Keyboard::Instance->Clear();
        focusEvent.EventId = Events::OsFocusLost;
      }
      SendWindowEvent(focusEvent, false);

      return MessageHandled;
    }

    // Device has been added so a controller may have been
    // plugged in 
    case WM_DEVICECHANGE:
    {
      ObjectEvent e(this);
      Z::gEngine->DispatchEvent(Events::OsDeviceAdded, &e);
      ScanRawInputDevices((uint)mWindowHandle);
      return MessageHandled;
    }

    // Raw input data has been sent
    case WM_INPUT:
    {
      RawInputMessage(wParam, lParam);
      return MessageHandled;
    }

    // Files have been dropped on this window
    case WM_DROPFILES:
    {
      HDROP fileDrop = (HDROP)wParam;

      // Get the drop mouse position
      IntVec2 mouseDropPosition;
      DragQueryPoint(fileDrop, (POINT*)&mouseDropPosition);

      OsMouseDropEvent mouseDrop;
      FillMouseEventData(mouseDropPosition, MouseButtons::None, mouseDrop);

      // Passing 0xFFFFFFFF will request the number of files.
      uint itemCount = DragQueryFile(fileDrop, 0xFFFFFFFF, 0, 0);

      // Get all the file names
      wchar_t buffer[MAX_PATH + 1] = {0};
      for(uint i = 0; i < itemCount; ++i)
      {
        //Get the file at this index and add to the list
        DragQueryFile((HDROP)wParam, i, buffer, MAX_PATH);
        mouseDrop.Files.PushBack(Narrow(buffer).c_str());
      }

      // Clean up the drag
      DragFinish((HDROP)wParam);

      mouseDrop.EventId = Events::OsMouseFileDrop;
      SendMouseDropEvent(mouseDrop, false);

      return MessageHandled;
    }

    case WM_TIMER:
    {
      Z::gEngine->Update();
      return MessageHandled;
    }

    // Start resizing
    case WM_ENTERSIZEMOVE:
    {
      //Enter Blocking, See WindowsSystem Update 
      mSystem->mState = WindowsSystemUpdateState::SystemBlocking;
      SetTimer(mWindowHandle, 1, 0, nullptr);
      return MessageHandled;
    }

    // Stop resizing
    case WM_EXITSIZEMOVE:
    {
      //Exit Blocking, See WindowsSystem Update 
      mSystem->mState = WindowsSystemUpdateState::Normal;
      KillTimer(mWindowHandle, 1);

      RECT rect = GetDesktopClientRect();
      IntVec2 newSize  = IntVec2(rect.right - rect.left, rect.bottom - rect.top);

      if(newSize != mClientSize)
      {
        mClientSize = newSize;
        OsWindowEvent sizeEvent;
        sizeEvent.ClientSize = mClientSize;
        this->DispatchEvent(Events::OsResized, &sizeEvent);
      }

      return MessageHandled;
    }

    // Returning 0 from WM_NCCALCSIZE removes the window border
    case WM_NCCALCSIZE:
      if (mBorderless)
        return 0;
      else
        return DefWindowProc(hwnd, messageId, wParam, lParam);

    case WM_GETMINMAXINFO:
    {
      MINMAXINFO* minMaxInfo =  (MINMAXINFO*)lParam;

      // Set min size
      minMaxInfo->ptMinTrackSize.x = mMinSize.x;
      minMaxInfo->ptMinTrackSize.y = mMinSize.y;

      if (mBorderless)
      {
        // Get the monitor this window is closest to
        HMONITOR appMonitor = MonitorFromWindow(mWindowHandle, MONITOR_DEFAULTTONEAREST);

        // Get the monitor information
        MONITORINFO monitorInfo = { sizeof(monitorInfo) };
        GetMonitorInfo(appMonitor, &monitorInfo);

        RECT monitorRect = monitorInfo.rcMonitor;
        RECT rect = monitorInfo.rcWork;

        minMaxInfo->ptMaxPosition.x = rect.left - monitorRect.left;
        minMaxInfo->ptMaxPosition.y = rect.top - monitorRect.top;
        minMaxInfo->ptMaxSize.x = rect.right - rect.left;
        minMaxInfo->ptMaxSize.y = rect.bottom - rect.top;
      }

      return MessageHandled;
    }

    // User has typed text
    case WM_CHAR:
    {
      KeyboardTextEvent textEvent = KeyboardTextEvent(Utf16ToUtf8(wParam));
      textEvent.EventId = Events::OsKeyTyped;
      SendKeyboardTextEvent(textEvent, false);
      return MessageHandled;
    }

    // Key has been pressed
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
      KeyState::Enum keyState = KeyState::Up;
      bool notrepeated = true;

      Keys::Enum key = TranslateKeyCode(wParam);

      KeyboardEvent keyEvent;
      keyEvent.OsKey = wParam;
      FillKeyboardEvent(key, keyState, keyEvent);
      SendKeyboardEvent(keyEvent, false);

      return MessageHandled;
    }

    // Key has been released
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
      bool notRepeated = (lParam & (1 << 30)) == 0;

      KeyState::Enum keyState = KeyState::Down;
      
      if(!notRepeated)
      {
        keyState = KeyState::Repeated;
      }

      Keys::Enum  key = TranslateKeyCode(wParam);

      KeyboardEvent keyEvent;
      keyEvent.OsKey = wParam;
      FillKeyboardEvent(key, keyState, keyEvent);
      SendKeyboardEvent(keyEvent, false);

      return MessageHandled;
    }

    // Mouse Button Up
    case WM_LBUTTONUP:
    {
      OsMouseEvent mouseEvent;
      FillMouseEventData(PositionFromLParam(lParam), MouseButtons::Left, mouseEvent);
      SendMouseEvent(mouseEvent, Events::OsMouseUp);
      return MessageHandled;
    }
    case WM_RBUTTONUP:
    {
      OsMouseEvent mouseEvent;
      FillMouseEventData(PositionFromLParam(lParam), MouseButtons::Right, mouseEvent);
      SendMouseEvent(mouseEvent, Events::OsMouseUp);
      return MessageHandled;
    }
    case WM_MBUTTONUP:
    {
      OsMouseEvent mouseEvent;
      FillMouseEventData(PositionFromLParam(lParam), MouseButtons::Middle, mouseEvent);
      SendMouseEvent(mouseEvent, Events::OsMouseUp);
      return MessageHandled;
     }
    case WM_XBUTTONUP:
    {
      OsMouseEvent mouseEvent;
      FillMouseEventData(PositionFromLParam(lParam), ButtonFromWParam(wParam), mouseEvent);
      SendMouseEvent(mouseEvent, Events::OsMouseUp);
      return MessageHandled;
    }

    // Mouse Button Down
    case WM_LBUTTONDOWN:
    {
      OsMouseEvent mouseEvent;
      FillMouseEventData(PositionFromLParam(lParam), MouseButtons::Left, mouseEvent);
      SendMouseEvent(mouseEvent, Events::OsMouseDown);
      return MessageHandled;
    }
    case WM_RBUTTONDOWN:
    {
      OsMouseEvent mouseEvent;
      FillMouseEventData(PositionFromLParam(lParam), MouseButtons::Right, mouseEvent);
      SendMouseEvent(mouseEvent, Events::OsMouseDown);
      return MessageHandled;
    }
    case WM_MBUTTONDOWN:
    {
      OsMouseEvent mouseEvent;
      FillMouseEventData(PositionFromLParam(lParam), MouseButtons::Middle, mouseEvent);
      SendMouseEvent(mouseEvent, Events::OsMouseDown);
      return MessageHandled;
    }
    case WM_XBUTTONDOWN:
    {
      OsMouseEvent mouseEvent;
      FillMouseEventData(PositionFromLParam(lParam), ButtonFromWParam(wParam), mouseEvent);
      SendMouseEvent(mouseEvent, Events::OsMouseDown);
      return MessageHandled;
    }

    // Mouse has moved on the window
    case WM_MOUSEMOVE:
    {
      OsMouseEvent mouseEvent;
      FillMouseEventData(PositionFromLParam(lParam), MouseButtons::None, mouseEvent);
      
      // If the mouse is trapped, we either need to ignore the move back message, or just tell the mouse to move back
      if(mMouseTrapped)
      {
        // Keep setting mouse clip to the main window
        RECT clientRect = GetDesktopClientRect();
        ::ClipCursor(&clientRect);

        // Invisible cursor in MouseTrap mode
        mCursor = nullptr;

        DWORD messagePos = GetMessagePos();
        POINTS cursorScreen = MAKEPOINTS(messagePos);

        // If the mouse is moving to the trap position, set its position back to the center
        POINT mouseTrapPointScreen = GetMouseTrapScreenPosition();
        if(cursorScreen.x != mouseTrapPointScreen.x || cursorScreen.y != mouseTrapPointScreen.y)
        {
          SetCursorPos(mouseTrapPointScreen.x, mouseTrapPointScreen.y);
        }
        else
        {
          mouseEvent.IsTrapMoveBack = true;
        }
      }

      SendMouseEvent(mouseEvent, Events::OsMouseMove);

      // Set the current cursor
      SetCursor(mCursor);

      return MessageHandled;
    }

    // Mouse Wheel Changed
    case WM_MOUSEWHEEL:
    {
      // Mouse Wheel Message not in client coordinates
      IntVec2 screenPoint = PositionFromLParam(lParam);
      IntVec2 localPoint = LocalScreenToClient(mWindowHandle, screenPoint);

      OsMouseEvent mouseEvent;
      FillMouseEventData(localPoint, MouseButtons::None, mouseEvent);

      int scrollAmount = GET_WHEEL_DELTA_WPARAM(wParam);
      mouseEvent.ScrollMovement.y = scrollAmount / (float)WHEEL_DELTA;


      SendMouseEvent(mouseEvent, Events::OsMouseScroll);
      return MessageHandled;
    }

    // Mouse Wheel Changed horizontal
    case WM_MOUSEHWHEEL:
    {
      // Mouse Wheel Message not in client coordinates
      IntVec2 screenPoint = PositionFromLParam(lParam);
      IntVec2 localPoint = LocalScreenToClient(mWindowHandle, screenPoint);

      OsMouseEvent mouseEvent;
      FillMouseEventData(localPoint, MouseButtons::None, mouseEvent);

      mouseEvent.ScrollMovement.x = HIWORD(wParam) / (float)WHEEL_DELTA;


      SendMouseEvent(mouseEvent, Events::OsMouseScroll);
      return MessageHandled;
    }

    // Default catch all
    default:
    {
      // 
      if(messageId == mTaskBarButtonCreated)
      {
        //Create the task bar button for loading progress
        DWORD dwMajor = LOBYTE(LOWORD(GetVersion()));
        DWORD dwMinor = HIBYTE(LOWORD(GetVersion()));
        if( dwMajor > 6 || ( dwMajor == 6 && dwMinor > 0 ) )
        {
          CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_ALL,
            __uuidof(ITaskbarList3), (LPVOID*)&mTaskBar);
        }
      }
    }
  }

  // Let the default window procedure handle the message 
  return DefWindowProc(hwnd, messageId, wParam, lParam);
}

//------------------------------------------------------------ WindowsShellSystem
WindowsShellSystem::WindowsShellSystem() :
  mIsUpdating(false)
{
  mState = WindowsSystemUpdateState::Normal;
  mMainWindow = nullptr;
}

WindowsShellSystem::~WindowsShellSystem()
{
  RawInputShutdown();

  DeleteObjectsInContainer(mWindows);

  HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(nullptr);

  UnregisterWindowClass(hInstance);
}

void WindowsShellSystem::Initialize(SystemInitializer& initializer)
{
  ErrorSignaler::SetErrorHandler(WindowsErrorProcessHandler);

  String displayName = Os::GetVersionString();
  ZPrint("Os: %s\n", displayName.c_str());

  initializer.mEngine->AddSystemInterface(ZilchTypeId(OsShell), this);

  HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(nullptr);

  // Register the main windows class used by main window
  RegisterWindowClass(hInstance);
}

void WindowsShellSystem::Update()
{
  // Prevent recursion due to cases where call Z::gEngine->Update(),
  // such as in the WM_TIMER message handling
  if (mIsUpdating)
    return;

  mIsUpdating = true;
  RawInputUpdate();
  Keyboard* keyboard = Keyboard::GetInstance();
  keyboard->Update();

  if (mOsShellHook)
    mOsShellHook->HookUpdate();

  ProfileScopeTree("ShellSystem", "Engine", Color::Red);

  // Run the windows message pump. Sometimes (like with window dragging)
  // the default windows message pump will block, however we would 
  // like to continue rendering and processing. DirectX9 Graphics must be on the
  // same thread as the message pump so a multi threaded approach is complicated
  // and does not solve the problem. Instead a timer is set up allowing the
  // game to continue running while inside the DefaultWindowProc. But in that
  // case WindowsSystem should not reenter the pump.
  // This is controlled with the mState variable.
  if(mState == WindowsSystemUpdateState::Normal)
  {
    MSG messageInfo;
    while(PeekMessage(&messageInfo, nullptr, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&messageInfo);
      DispatchMessage(&messageInfo);
    }
  }

  // This is a special place to update for other systems like the
  // CEF WebBrowser that may cause the message pump to run.
  Event toSend;
  DispatchEvent(Events::OsShellUpdate, &toSend);

  mIsUpdating = false;
}

cstr WindowsShellSystem::GetName()
{
  return "WindowsSystem";
}

String WindowsShellSystem::GetOsName()
{
  return "Windows";
}

WindowsOsWindow* WindowsShellSystem::FindWindowAt(IntVec2Param position)
{
  HWND windowHandle = WindowFromPoint(*(POINT*)&position);

  if(windowHandle == nullptr)
    return nullptr;

  // Is this window a zero window?
  // Check its class name 
  wchar_t className[MAX_PATH];
  GetClassName(windowHandle, className, MAX_PATH);
  if(wcscmp(className, cWindowClassName) != 0)
    return nullptr;

  return (WindowsOsWindow*)PointerFromWindow(windowHandle);
}

PixelRect WindowsShellSystem::GetDesktopRect()
{
  POINT ptZero = { 0 };
  HMONITOR hmonPrimary = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
  MONITORINFO monitorinfo = { 0 };
  monitorinfo.cbSize = sizeof(monitorinfo);
  GetMonitorInfo(hmonPrimary, &monitorinfo);
  return FromRECT(monitorinfo.rcWork);
}

WindowsOsWindow* WindowsShellSystem::CreateOsWindow(StringParam windowName, IntVec2Param windowSize, IntVec2Param windowPos,
                                                    OsWindow* parentWindowOs, WindowStyleFlags::Enum flags)
{
  WindowsOsWindow* osWindow = new WindowsOsWindow();

  if(flags & WindowStyleFlags::ClientOnly)
    osWindow->mBorderless = true;

  // Get the parent window
  WindowsOsWindow* parentWindow = (WindowsOsWindow*)parentWindowOs;
  HWND parentWindowHwnd = nullptr;
  
  if(parentWindow)
    parentWindowHwnd = parentWindow->mWindowHandle;

  // Get HINSTANCE
  HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(nullptr);

  osWindow->mSystem = this;
  osWindow->mWindowStyle = flags;
  osWindow->mIsMainWindow = (WindowStyleFlags::MainWindow & flags) != 0;
  osWindow->mParent = parentWindow;

  // Translate the style flags
  DWORD style = Win32StyleFromWindowStyle(flags);
  DWORD exStyle = GetWin32ExStyle(flags);

  // Create the window
  HWND windowHandle = CreateWindowEx(exStyle, cWindowClassName, Widen(windowName).c_str(), style,
                                     CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                     parentWindowHwnd, nullptr, hInstance, (LPVOID)osWindow);

  if(windowHandle == nullptr)
    FatalEngineError("Failed to create application window");

  osWindow->SetPosition(windowPos);
  osWindow->SetClientSize(windowSize);

  SetWindowPos(windowHandle, nullptr, 0, 0, 0, 0,
               SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

  // Set Handle
  osWindow->mWindowHandle = windowHandle;
  UpdateWindow(windowHandle);

  if(WindowStyleFlags::MainWindow & flags)
    mMainWindow = windowHandle;

  osWindow->mTaskBarButtonCreated = RegisterWindowMessageW(L"TaskbarButtonCreated");
  osWindow->mCursor = LoadCursor(nullptr, IDC_ARROW);
  DragAcceptFiles(windowHandle, TRUE);

  osWindow->mPosition = windowPos;
  osWindow->mClientSize = windowSize;

  ScanRawInputDevices((uint)windowHandle);

  mWindows.PushBack(osWindow);

  return osWindow;
}

ByteColor WindowsShellSystem::GetColorAtMouse()
{
  // Get the cursorPos
  POINT cursorPos;
  GetCursorPos(&cursorPos);

  HDC dc = GetDC(nullptr);
  COLORREF color = GetPixel(dc, cursorPos.x, cursorPos.y);
  ReleaseDC(nullptr, dc);

  //COLORREF is 0x00BBGGRR
  return ByteColorRGBA( GetRValue(color),  GetGValue(color),  GetBValue(color), 0xFF);
}

bool WindowsShellSystem::IsClipboardText()
{
  return (IsClipboardFormatAvailable(CF_UNICODETEXT) != 0);
}

String WindowsShellSystem::GetClipboardText()
{
  return ShellGetClipboardText(nullptr);
}

void WindowsShellSystem::SetClipboardText(StringParam text)
{
  ShellSetClipboardText(nullptr, text);
}

bool WindowsShellSystem::IsClipboardImageAvailable()
{
  return ShellIsClipboardImageAvailable(nullptr);
}

bool WindowsShellSystem::GetClipboardImage(Image* image)
{
  return ShellGetClipboardImage(nullptr, image);
}

bool WindowsShellSystem::GetWindowImage(Image* image)
{
  return ShellGetWindowImage(mMainWindow, image);
}

void WindowsShellSystem::OpenFile(FileDialogConfig& config)
{
  //Function handles both Open and Save with flag
  FileDialog(mMainWindow, config, true);
}

void WindowsShellSystem::SaveFile(FileDialogConfig& config)
{
  //Function handles both Open and Save with flag
  FileDialog(mMainWindow, config, false);
}

void WindowsShellSystem::ShowMessageBox(StringParam title, StringParam message)
{
  MessageBoxA(mMainWindow, message.c_str(), title.c_str(), MB_OK);
}

size_t WindowsShellSystem::GetWindowCount()
{
  return mWindows.Size();
}

OsWindow* WindowsShellSystem::GetWindow(size_t index)
{
  if (index > mWindows.Size())
  {
    DoNotifyException("Shell", "Invalid window index");
    return nullptr;
  }
  return mWindows[index];
}

}//namespace Zero
