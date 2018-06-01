////////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2018, DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------- Shell
Shell::Shell() :
  mCursor(Cursor::Arrow),
  mMainWindow(nullptr),
  mUserData(nullptr)
{
  Error("Not implemented");
}

Shell::~Shell()
{
  Error("Not implemented");
}

String Shell::GetOsName()
{
  Error("Not implemented");
  return String();
}

uint Shell::GetScrollLineCount()
{
  Error("Not implemented");
  return -1;
}

IntRect Shell::GetPrimaryMonitorRectangle()
{
  Error("Not implemented");
  return IntRect();
}

IntVec2 Shell::GetPrimaryMonitorSize()
{
  Error("Not implemented");
  return IntVec2::cZero;
}

ByteColor Shell::GetColorAtMouse()
{
  Error("Not implemented");
  return -1;
}

void Shell::SetMonitorCursorClip(const IntRect& monitorRectangle)
{
  Error("Not implemented");
}

void Shell::ClearMonitorCursorClip()
{
  Error("Not implemented");
}

Cursor::Enum Shell::GetMouseCursor()
{
  Error("Not implemented");
  return mCursor;
}

void Shell::SetMonitorCursorPosition(Math::IntVec2Param monitorPosition)
{
  Error("Not implemented");
}

Math::IntVec2 Shell::GetMonitorCursorPosition()
{
  Error("Not implemented");
  return IntVec2::cZero;
}

bool Shell::IsKeyDown(Keys::Enum key)
{
  Error("Not implemented");
  return false;
}

bool Shell::IsMouseDown(MouseButtons::Enum button)
{
  Error("Not implemented");
  return false;
}

void Shell::SetMouseCursor(Cursor::Enum cursor)
{
  Error("Not implemented");
}

ShellWindow* Shell::FindWindowAt(Math::IntVec2Param monitorPosition)
{
  Error("Not implemented");
  return nullptr;
}

bool Shell::IsClipboardText()
{
  Error("Not implemented");
  return false;
}

String Shell::GetClipboardText()
{
  Error("Not implemented");
  return String();
}

void Shell::SetClipboardText(StringParam text)
{
  Error("Not implemented");
}

bool Shell::IsClipboardImage()
{
  Error("Not implemented");
  return false;
}

bool Shell::GetClipboardImage(Image* image)
{
  Error("Not implemented");
  return false;
}

bool Shell::GetPrimaryMonitorImage(Image* image)
{
  Error("Not implemented");
  return false;
}

bool Shell::OpenFile(FileDialogInfo& config)
{
  Error("Not implemented");
  return false;
}

bool Shell::SaveFile(FileDialogInfo& config)
{
  Error("Not implemented");
  return false;
}

void Shell::ShowMessageBox(StringParam title, StringParam message)
{
  Error("Not implemented");
}

void Shell::Update()
{
  Error("Not implemented");
}

const Array<PlatformInputDevice>& Shell::ScanInputDevices()
{
  Error("Not implemented");
  return mInputDevices;
}

//-------------------------------------------------------------------- ShellWindow
ShellWindow::ShellWindow(
  Shell* shell,
  StringParam windowName,
  Math::IntVec2Param clientSize,
  Math::IntVec2Param monitorClientPos,
  ShellWindow* parentWindow,
  WindowStyleFlags::Enum flags) :
  mShell(shell),
  mMinClientSize(IntVec2(10, 10)),
  mParent(parentWindow),
  mHandle(nullptr),
  mStyle(flags),
  mProgress(0),
  mClientSize(clientSize),
  mClientMousePosition(IntVec2(-1, -1)),
  mUserData(nullptr),
  mOnClose(nullptr),
  mOnFocusChanged(nullptr),
  mOnMouseDropFiles(nullptr),
  mOnFrozenUpdate(nullptr),
  mOnClientSizeChanged(nullptr),
  mOnMinimized(nullptr),
  mOnRestored(nullptr),
  mOnTextTyped(nullptr),
  mOnKeyDown(nullptr),
  mOnKeyUp(nullptr),
  mOnMouseDown(nullptr),
  mOnMouseUp(nullptr),
  mOnMouseMove(nullptr),
  mOnMouseScrollY(nullptr),
  mOnMouseScrollX(nullptr),
  mOnDevicesChanged(nullptr),
  mOnRawMouseChanged(nullptr),
  mOnInputDeviceChanged(nullptr)
{
  Error("Not implemented");
}

ShellWindow::~ShellWindow()
{
  Error("Not implemented");
}

void ShellWindow::Destroy()
{
  Error("Not implemented");
}

IntRect ShellWindow::GetMonitorClientRectangle()
{
  Error("Not implemented");
  return IntRect();
}

void ShellWindow::SetMonitorClientRectangle(const IntRect& monitorRectangle)
{
  Error("Not implemented");
}

IntRect ShellWindow::GetMonitorBorderedRectangle()
{
  Error("Not implemented");
  return IntRect();
}

void ShellWindow::SetMonitorBorderedRectangle(const IntRect& monitorRectangle)
{
  Error("Not implemented");
}

IntVec2 ShellWindow::GetMinClientSize()
{
  Error("Not implemented");
  return IntVec2::cZero;
}

void ShellWindow::SetMinClientSize(Math::IntVec2Param minSize)
{
  Error("Not implemented");
}

ShellWindow* ShellWindow::GetParent()
{
  Error("Not implemented");
  return nullptr;
}

IntVec2 ShellWindow::MonitorToClient(Math::IntVec2Param monitorPosition)
{
  Error("Not implemented");
  return IntVec2::cZero;
}

IntVec2 ShellWindow::MonitorToBordered(Math::IntVec2Param monitorPosition)
{
  Error("Not implemented");
  return IntVec2::cZero;
}

IntVec2 ShellWindow::ClientToMonitor(Math::IntVec2Param clientPosition)
{
  Error("Not implemented");
  return IntVec2::cZero;
}

IntVec2 ShellWindow::ClientToBordered(Math::IntVec2Param clientPosition)
{
  Error("Not implemented");
  return IntVec2::cZero;
}

IntVec2 ShellWindow::BorderedToMonitor(Math::IntVec2Param borderedPosition)
{
  Error("Not implemented");
  return IntVec2::cZero;
}

IntVec2 ShellWindow::BorderedToClient(Math::IntVec2Param borderedPosition)
{
  Error("Not implemented");
  return IntVec2::cZero;
}

WindowStyleFlags::Enum ShellWindow::GetStyle()
{
  Error("Not implemented");
  return WindowStyleFlags::NotVisible;
}

void ShellWindow::SetStyle(WindowStyleFlags::Enum style)
{
  Error("Not implemented");
}

bool ShellWindow::GetVisible()
{
  Error("Not implemented");
  return false;
}

void ShellWindow::SetVisible(bool visible)
{
  Error("Not implemented");
}

String ShellWindow::GetTitle()
{
  Error("Not implemented");
  return String();
}

void ShellWindow::SetTitle(StringParam title)
{
  Error("Not implemented");
}

WindowState::Enum ShellWindow::GetState()
{
  Error("Not implemented");
  return WindowState::Fullscreen;
}

void ShellWindow::SetState(WindowState::Enum windowState)
{
  Error("Not implemented");
}

void ShellWindow::SetMouseCapture(bool capture)
{
  Error("Not implemented");
}

bool ShellWindow::GetMouseCapture()
{
  Error("Not implemented");
  return false;
}

void ShellWindow::TakeFocus()
{
  Error("Not implemented");
}

bool ShellWindow::HasFocus()
{
  Error("Not implemented");
  return false;
}

bool ShellWindow::GetImage(Image* image)
{
  Error("Not implemented");
  return false;
}

void ShellWindow::Close()
{
  Error("Not implemented");
}

void ShellWindow::ManipulateWindow(WindowBorderArea::Enum area)
{
  Error("Not implemented");
}

float ShellWindow::GetProgress()
{
  Error("Not implemented");
  return 0.0f;
}

void ShellWindow::SetProgress(ProgressType::Enum progressType, float progress)
{
  Error("Not implemented");
}

void ShellWindow::PlatformSpecificFixup()
{
  Error("Not implemented");
}

}// namespace Zero
