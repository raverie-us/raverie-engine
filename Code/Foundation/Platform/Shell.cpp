// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Foundation/Platform/PlatformCommunication.hpp"

namespace Zero
{
Shell* Shell::sInstance;
String gClipboardText;

Shell::Shell() : mCursor(Cursor::Arrow), mMainWindow(nullptr)
{
  sInstance = this;
}

Shell::~Shell()
{
}

IntVec2 Shell::GetPrimaryMonitorSize()
{
  return cMinimumMonitorSize;
}

ByteColor Shell::GetColorAtMouse()
{
  return 0;
}

bool Shell::IsKeyDown(Keys::Enum key)
{
  return mKeyState[key];
}

bool Shell::IsMouseDown(MouseButtons::Enum button)
{
  return mMouseState[button];
}

Cursor::Enum Shell::GetMouseCursor()
{
  return mCursor;
}

void Shell::SetMouseCursor(Cursor::Enum cursor)
{
  mCursor = cursor;
  ImportMouseSetCursor(cursor);
}

void Shell::OpenFile(FileDialogInfo& config)
{
  StringBuilder acceptExtensions;
  forRange (FileDialogFilter& filter, config.mSearchFilters)
  {
    if (acceptExtensions.GetSize() != 0)
      acceptExtensions.Append(',');

    // Filter out all the wildcard stars '*' since (currently cannot use them)
    forRange (Rune rune, filter.mFilter)
    {
      if (rune == ';')
        acceptExtensions.Append(',');
      else if (rune != '*')
        acceptExtensions.Append(rune);
    }
  }

  String accept = acceptExtensions.ToString();
  ImportOpenFileDialog(&config, config.mMultiple, accept.c_str());
}

void ZeroExportNamed(ExportOpenFileDialogAdd)(void* dialog, const char* filePath) {
  FileDialogInfo& config = *(FileDialogInfo*)dialog;
  config.mFiles.PushBack(filePath);
}

void ZeroExportNamed(ExportOpenFileDialogFinish)(void* dialog) {
  FileDialogInfo& config = *(FileDialogInfo*)dialog;
  config.mCallback(config.mFiles, config.mUserData);
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
                         WindowStyleFlags::Enum flags,
                         WindowState::Enum state) :
    mShell(shell),
    mMinClientSize(IntVec2(10, 10)),
    mHandle(nullptr),
    mStyle(flags),
    mClientSize(cMinimumMonitorSize),
    mClientMousePosition(IntVec2(-1, -1)),
    mUserData(nullptr),
    mOnClose(nullptr),
    mOnFocusChanged(nullptr),
    mOnClientSizeChanged(nullptr),
    mOnDevicesChanged(nullptr),
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

bool ShellWindow::HasOwnMinMaxExitButtons()
{
  return !mStyle.IsSet(WindowStyleFlags::ClientOnly);
}

} // namespace Zero
