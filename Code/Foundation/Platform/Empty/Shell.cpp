// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
Shell* Shell::sInstance;
String gClipboardText;

Shell::Shell() : mCursor(Cursor::Arrow), mMainWindow(nullptr), mUserData(nullptr), mOnCopy(nullptr), mOnPaste(nullptr)
{
  sInstance = this;
}

Shell::~Shell()
{
}

String Shell::GetOsName()
{
  return "Empty";
}

uint Shell::GetScrollLineCount()
{
  return 3;
}

IntRect Shell::GetPrimaryMonitorRectangle()
{
  return IntRect(0, 0, cMinimumMonitorSize.x, cMinimumMonitorSize.y);
}

IntVec2 Shell::GetPrimaryMonitorSize()
{
  return cMinimumMonitorSize;
}

ByteColor Shell::GetColorAtMouse()
{
  return 0;
}

void Shell::SetMonitorCursorClip(const IntRect& monitorRectangle)
{
}

void Shell::ClearMonitorCursorClip()
{
}

Cursor::Enum Shell::GetMouseCursor()
{
  return mCursor;
}

void Shell::SetMonitorCursorPosition(Math::IntVec2Param monitorPosition)
{
}

Math::IntVec2 Shell::GetMonitorCursorPosition()
{
  return IntVec2::cZero;
}

bool Shell::IsKeyDown(Keys::Enum key)
{
  return false;
}

bool Shell::IsMouseDown(MouseButtons::Enum button)
{
  return false;
}

void Shell::SetMouseCursor(Cursor::Enum cursor)
{
  mCursor = cursor;
}

bool Shell::GetPrimaryMonitorImage(Image* image)
{
  return false;
}

void Shell::OpenFile(FileDialogInfo& config)
{
}

void Shell::SaveFile(FileDialogInfo& config)
{
}

void Shell::ShowMessageBox(StringParam title, StringParam message)
{
}

void Shell::Update()
{
}

const Array<PlatformInputDevice>& Shell::ScanInputDevices()
{
  return mInputDevices;
}

// ShellWindow
ShellWindow::ShellWindow(Shell* shell,
                         StringParam windowName,
                         Math::IntVec2Param clientSize,
                         Math::IntVec2Param monitorClientPos,
                         ShellWindow* parentWindow,
                         WindowStyleFlags::Enum flags,
                         WindowState::Enum state) :
    mShell(shell),
    mMinClientSize(IntVec2(10, 10)),
    mParent(parentWindow),
    mHandle(nullptr),
    mStyle(flags),
    mProgress(0),
    mClientSize(cMinimumMonitorSize),
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
}

ShellWindow::~ShellWindow()
{
}

void ShellWindow::Destroy()
{
}

IntRect ShellWindow::GetMonitorClientRectangle()
{
  return IntRect(0, 0, cMinimumMonitorSize.x, cMinimumMonitorSize.y);
}

void ShellWindow::SetMonitorClientRectangle(const IntRect& monitorRectangle)
{
}

IntRect ShellWindow::GetMonitorBorderedRectangle()
{
  return IntRect(0, 0, cMinimumMonitorSize.x, cMinimumMonitorSize.y);
}

void ShellWindow::SetMonitorBorderedRectangle(const IntRect& monitorRectangle)
{
}

IntVec2 ShellWindow::GetMinClientSize()
{
  return mMinClientSize;
}

void ShellWindow::SetMinClientSize(Math::IntVec2Param minSize)
{
  mMinClientSize = minSize;
}

ShellWindow* ShellWindow::GetParent()
{
  return nullptr;
}

IntVec2 ShellWindow::MonitorToClient(Math::IntVec2Param monitorPosition)
{
  return monitorPosition;
}

IntVec2 ShellWindow::MonitorToBordered(Math::IntVec2Param monitorPosition)
{
  return monitorPosition;
}

IntVec2 ShellWindow::ClientToMonitor(Math::IntVec2Param clientPosition)
{
  return clientPosition;
}

IntVec2 ShellWindow::ClientToBordered(Math::IntVec2Param clientPosition)
{
  return clientPosition;
}

IntVec2 ShellWindow::BorderedToMonitor(Math::IntVec2Param borderedPosition)
{
  return borderedPosition;
}

IntVec2 ShellWindow::BorderedToClient(Math::IntVec2Param borderedPosition)
{
  return borderedPosition;
}

WindowStyleFlags::Enum ShellWindow::GetStyle()
{
  return WindowStyleFlags::NotVisible;
}

void ShellWindow::SetStyle(WindowStyleFlags::Enum style)
{
}

bool ShellWindow::GetVisible()
{
  return false;
}

void ShellWindow::SetVisible(bool visible)
{
}

String ShellWindow::GetTitle()
{
  return "Window";
}

void ShellWindow::SetTitle(StringParam title)
{
}

WindowState::Enum ShellWindow::GetState()
{
  return WindowState::Fullscreen;
}

void ShellWindow::SetState(WindowState::Enum windowState)
{
}

void ShellWindow::SetMouseCapture(bool capture)
{
}

bool ShellWindow::GetMouseCapture()
{
  return false;
}

void ShellWindow::TakeFocus()
{
}

bool ShellWindow::HasFocus()
{
  return false;
}

bool ShellWindow::GetImage(Image* image)
{
  return false;
}

void ShellWindow::Close()
{
}

float ShellWindow::GetProgress()
{
  return mProgress;
}

void ShellWindow::SetProgress(ProgressType::Enum progressType, float progress)
{
  mProgress = progress;
}

void ShellWindow::PlatformSpecificFixup()
{
}

bool ShellWindow::HasOwnMinMaxExitButtons()
{
  return !mStyle.IsSet(WindowStyleFlags::ClientOnly);
}

} // namespace Zero
