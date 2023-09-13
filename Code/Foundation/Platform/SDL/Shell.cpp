// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
Shell* Shell::sInstance;

struct DropFileInfo
{
  bool mBeganDropFiles = false;
  Array<String> mDropFiles;
};

struct ShellPrivateData
{
  HashMap<Uint32, DropFileInfo> mDropInfos;
  Array<SDL_Cursor*> mSDLCursors;
};

static const char* cShellWindow = "ShellWindow";
SDL_Window* gSdlMainWindow = nullptr;

// In SDL 'global' is synonymous with 'monitor' space and 'relative' means
// 'client' space.

Keys::Enum SDLScancodeToKey(SDL_Scancode code)
{
  switch (code)
  {
#define ProcessInput(Scancode, Keycode, ZeroValue)                                                                     \
  case Scancode:                                                                                                       \
    return ZeroValue;
#include "Keys.inl"
#undef ProcessInput
  default:
    break;
  }

  switch (code)
  {
  // Treat the gui keys as if they are control.
  case SDL_SCANCODE_LGUI:
  case SDL_SCANCODE_RGUI:
    return Keys::Control;

  // Keys.inl already handles all the right versions.
  case SDL_SCANCODE_RCTRL:
    return Keys::Control;
  case SDL_SCANCODE_RSHIFT:
    return Keys::Shift;
  case SDL_SCANCODE_RALT:
    return Keys::Alt;
  default:
    break;
  }

  return Keys::Unknown;
}

SDL_Scancode KeyToSDLScancode(Keys::Enum key)
{
  switch (key)
  {
#define ProcessInput(Scancode, Keycode, ZeroValue)                                                                     \
  case ZeroValue:                                                                                                      \
    return Scancode;
#include "Keys.inl"
#undef ProcessInput
  default:
    break;
  }
  return SDL_SCANCODE_UNKNOWN;
}

Keys::Enum SDLKeycodeToKey(SDL_Keycode code)
{
  switch (code)
  {
#define ProcessInput(Scancode, Keycode, ZeroValue)                                                                     \
  case Keycode:                                                                                                        \
    return ZeroValue;
#include "Keys.inl"
#undef ProcessInput
  default:
    break;
  }

  switch (code)
  {
  // Treat the gui keys as if they are control.
  case SDLK_LGUI:
  case SDLK_RGUI:
    return Keys::Control;

  // Keys.inl already handles all the right versions.
  case SDLK_RCTRL:
    return Keys::Control;
  case SDLK_RSHIFT:
    return Keys::Shift;
  case SDLK_RALT:
    return Keys::Alt;
  }
  return Keys::Unknown;
}

bool SDLGetClipboardText(String* out)
{
  if (!SDL_HasClipboardText())
    return false;
  char* clipboardText = SDL_GetClipboardText();
  if (!clipboardText)
    return false;
  *out = clipboardText;
  SDL_free(clipboardText);
  return true;
}

#if !defined(ZeroPlatformNoShellOpenFile)
void Shell::OpenFile(FileDialogInfo& config)
{
  SDL_ShowSimpleMessageBox(
      SDL_MESSAGEBOX_WARNING, "Unsupported", "The file open dialog is not yet supported in SDL", nullptr);
  if (config.mCallback)
    config.mCallback(config.mFiles, config.mUserData);
}
#endif

#if !defined(ZeroPlatformNoShellSaveFile)
void Shell::SaveFile(FileDialogInfo& config)
{
  String downloads = FilePath::Combine(GetUserDocumentsApplicationDirectory(), "Downloads");
  CreateDirectoryAndParents(downloads);

  String filePath = FilePath::Combine(downloads, config.DefaultFileName);
  if (FileExists(filePath) || DirectoryExists(filePath))
  {
    String id = "_" + ToString(GenerateUniqueId64());
    String fileName;
    StringRange dot = config.DefaultFileName.FindLastOf(".");
    if (!dot.Empty())
      fileName = BuildString(config.DefaultFileName.SubString(config.DefaultFileName.Begin(), dot.Begin()),
                             id,
                             ".",
                             config.DefaultFileName.SubString(dot.End(), config.DefaultFileName.End()));
    else
      fileName = config.DefaultFileName + id;

    filePath = FilePath::Combine(downloads, fileName);
  }

  config.mFiles.PushBack(filePath);
  if (config.mCallback)
    config.mCallback(config.mFiles, config.mUserData);
}
#endif

void Shell::ShowMessageBox(StringParam title, StringParam message)
{
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title.c_str(), message.c_str(), nullptr);
}

void UpdateResize(ShellWindow* window, IntVec2Param clientSize)
{
  if (clientSize == window->mClientSize)
    return;

  window->mOnClientSizeChanged(clientSize, window);
  window->mClientSize = clientSize;
}

void Shell::Update()
{
  ZeroGetPrivateData(ShellPrivateData);
  ShellWindow* mainWindow = mMainWindow;

  // Some platforms don't sent all resize events, so handle that here.
  // For example, if you resize the window while loading in Emscripten, it
  // misses the resize.
  if (mainWindow)
    UpdateResize(mainWindow, mainWindow->GetClientSize());

  SDL_Event e;
  while (SDL_PollEvent(&e))
  {
    switch (e.type)
    {
    case SDL_QUIT:
    {
      if (mainWindow && mainWindow->mOnClose)
        mainWindow->mOnClose(mainWindow);
      break;
    }

    case SDL_WINDOWEVENT:
    {
      ShellWindow* window = GetShellWindowFromSDLId(e.window.windowID);
      if (!window)
        break;

      switch (e.window.event)
      {
      case SDL_WINDOWEVENT_FOCUS_GAINED:
      case SDL_WINDOWEVENT_FOCUS_LOST:

        if (window->mOnFocusChanged)
          window->mOnFocusChanged(e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED, window);
        break;

      case SDL_WINDOWEVENT_RESIZED:
      case SDL_WINDOWEVENT_SIZE_CHANGED:
        if (window->mOnClientSizeChanged)
          UpdateResize(window, IntVec2(e.window.data1, e.window.data2));
        break;
      }
      break;
    }

    case SDL_DROPBEGIN:
    case SDL_DROPCOMPLETE:
    case SDL_DROPFILE:
    {
      ShellWindow* window = GetShellWindowFromSDLId(e.drop.windowID);
      DropFileInfo& info = self->mDropInfos[e.drop.windowID];
      if (e.type == SDL_DROPBEGIN)
      {
        ErrorIf(info.mBeganDropFiles, "Got SDL_DROPBEGIN while another SDL_DROPBEGIN was in progress");
        info.mBeganDropFiles = true;
      }
      else if (e.type == SDL_DROPCOMPLETE)
      {
        ErrorIf(!info.mBeganDropFiles, "Got SDL_DROPCOMPLETE without a SDL_DROPBEGIN");
        info.mBeganDropFiles = false;
      }
      else if (e.type == SDL_DROPFILE)
      {
        info.mDropFiles.PushBack(e.drop.file);
      }

      // If this is a drop file without a begin, or it's a drop complete and
      // we're ending a drop...
      if (window && window->mOnMouseDropFiles && !info.mBeganDropFiles)
      {
        IntVec2 clientPosition = IntVec2::cZero;
        SDL_GetMouseState(&clientPosition.x, &clientPosition.y);

        window->mOnMouseDropFiles(clientPosition, info.mDropFiles, window);
        info.mDropFiles.Clear();
        SDL_free(e.drop.file);
      }
      break;
    }

    case SDL_KEYDOWN:
    {
      ShellWindow* window = GetShellWindowFromSDLId(e.key.windowID);
      Keys::Enum key = SDLKeycodeToKey(e.key.keysym.sym);

      if (window && window->mOnKeyDown)
        window->mOnKeyDown(key, e.key.keysym.scancode, e.key.repeat != 0, window);

#if !defined(ZeroPlatformNoClipboardEvents)
      // Handle paste explicitly to be more like a browser platform
      if (IsKeyDown(Keys::Control) && key == Keys::V && mOnPaste)
      {
        // SDL clipboard does not support images, but we may want to support url encoded images at some point
        ClipboardData data;
        data.mHasText = SDLGetClipboardText(&data.mText);
        mOnPaste(data, this);
      }

      // Handle copy explicitly to be more like a browser platform
      if (IsKeyDown(Keys::Control) && (key == Keys::C || key == Keys::X) && mOnCopy)
      {
        ClipboardData data;
        mOnCopy(data, key == Keys::X, this);
        ErrorIf(!data.mHasText && !data.mText.Empty(), "Clipboard Text was not empty, but HasText was not set");
        if (data.mHasText)
          SDL_SetClipboardText(data.mText.c_str());
        ErrorIf(data.mHasImage, "Copying image data not yet supported");
      }
#endif
      break;
    }

    case SDL_MOUSEWHEEL:
    {
      IntVec2 clientPosition = IntVec2::cZero;
      SDL_GetMouseState(&clientPosition.x, &clientPosition.y);

      // May need to handle SDL_MOUSEWHEEL_FLIPPED here
      ShellWindow* window = GetShellWindowFromSDLId(e.wheel.windowID);
      if (window)
      {
        // Currently we are using the sign of the scroll delta because the value
        // is mostly undefined per platform. We should probably refactor all
        // scroll wheel deltas to be in pixels scrolled.
        if (e.wheel.x && window->mOnMouseScrollX)
          window->mOnMouseScrollX(clientPosition, (float)Math::Sign(e.wheel.x), window);
        if (e.wheel.y && window->mOnMouseScrollY)
          window->mOnMouseScrollY(clientPosition, (float)Math::Sign(e.wheel.y), window);
      }
      break;
    }

    case SDL_JOYDEVICEADDED:
    case SDL_JOYDEVICEREMOVED:
    case SDL_AUDIODEVICEADDED:
    case SDL_AUDIODEVICEREMOVED:
    {
      if (mainWindow && mainWindow->mOnDevicesChanged)
        mainWindow->mOnDevicesChanged(mainWindow);
      break;
    }

    case SDL_JOYAXISMOTION:
    {
      Error("Not implemented");
      PlatformInputDevice* device = PlatformInputDeviceFromSDL(e.jaxis.which);
      if (device && mainWindow && mainWindow->mOnInputDeviceChanged)
        mainWindow->mOnInputDeviceChanged(*device, 0, Array<uint>(), DataBlock(), mainWindow);
      break;
    }
    default:
      break;
    }
  }
}

const Array<PlatformInputDevice>& Shell::ScanInputDevices()
{
  mInputDevices.Clear();

  return mInputDevices;
}

ShellWindow::ShellWindow(Shell* shell,
                         StringParam windowName,
                         Math::IntVec2Param clientSize,
                         Math::IntVec2Param monitorClientPos,
                         WindowStyleFlags::Enum flags,
                         WindowState::Enum state) :
    mShell(shell),
    mMinClientSize(IntVec2(10, 10)),
    mParent(parentWindow),
    mHandle(nullptr),
    mStyle(flags),
    mClientSize(clientSize),
    mClientMousePosition(IntVec2(-1, -1)),
    mCapture(false),
    mUserData(nullptr),
    mOnClose(nullptr),
    mOnFocusChanged(nullptr),
    mOnMouseDropFiles(nullptr),
    mOnClientSizeChanged(nullptr),
    mOnMouseDown(nullptr),
    mOnMouseUp(nullptr),
    mOnMouseScrollY(nullptr),
    mOnMouseScrollX(nullptr),
    mOnDevicesChanged(nullptr),
    mOnHitTest(nullptr),
    mOnInputDeviceChanged(nullptr)
{
  Uint32 sdlFlags = SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_OPENGL;
  if (flags & WindowStyleFlags::NotVisible)
    sdlFlags |= SDL_WINDOW_HIDDEN;
  if (flags & WindowStyleFlags::Resizable)
    sdlFlags |= SDL_WINDOW_RESIZABLE;
  if (flags & WindowStyleFlags::ClientOnly)
    sdlFlags |= SDL_WINDOW_BORDERLESS;

  if (!(flags & WindowStyleFlags::OnTaskBar))
    sdlFlags |= SDL_WINDOW_SKIP_TASKBAR;

  switch (state)
  {
  case WindowState::Minimized:
    sdlFlags |= SDL_WINDOW_MINIMIZED;
    break;

  case WindowState::Maximized:
    sdlFlags |= SDL_WINDOW_MAXIMIZED;
    break;

  case WindowState::Fullscreen:
    sdlFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    break;
  default:
    break;
  }

  // Is the width and height in client space? SDL doesn't say...
  SDL_Window* sdlWindow = SDL_CreateWindow(
      windowName.c_str(), monitorClientPos.x, monitorClientPos.y, clientSize.x, clientSize.y, sdlFlags);
  ErrorIf(sdlWindow == nullptr, "%s", SDL_GetError());
  mHandle = sdlWindow;

  SDL_SetWindowHitTest(sdlWindow, &ShellWindowSDLHitTest, this);

  SDL_SetWindowData(sdlWindow, cShellWindow, this);

  if (WindowStyleFlags::MainWindow & flags)
  {
    ErrorIf(shell->mMainWindow != nullptr, "Another main window already exists");
    shell->mMainWindow = this;
    gSdlMainWindow = sdlWindow;
  }
  shell->mWindows.PushBack(this);
}

ShellWindow::~ShellWindow()
{
  if (gSdlMainWindow == mHandle)
    gSdlMainWindow = nullptr;
  Destroy();
}

void ShellWindow::Destroy()
{
  if (!mHandle)
    return;

  if (mShell && mShell->mMainWindow == this)
    mShell->mMainWindow = nullptr;

  mShell->mWindows.EraseValue(this);

  SDL_DestroyWindow((SDL_Window*)mHandle);

  mHandle = nullptr;
}

IntRect ShellWindow::GetMonitorClientRectangle()
{
  IntRect monitorClientRectangle;
  SDL_GetWindowSize((SDL_Window*)mHandle, &monitorClientRectangle.SizeX, &monitorClientRectangle.SizeY);
  SDL_GetWindowPosition((SDL_Window*)mHandle, &monitorClientRectangle.X, &monitorClientRectangle.Y);
  return monitorClientRectangle;
}

void ShellWindow::SetMonitorClientRectangle(const IntRect& monitorRectangle)
{
  SDL_SetWindowPosition((SDL_Window*)mHandle, monitorRectangle.X, monitorRectangle.Y);
  SDL_SetWindowSize((SDL_Window*)mHandle, monitorRectangle.SizeX, monitorRectangle.SizeY);
  mClientSize = monitorRectangle.Size();
}

IntRect ShellWindow::GetMonitorBorderedRectangle()
{
  int top = 0;
  int left = 0;
  int bottom = 0;
  int right = 0;

  SDL_GetWindowBordersSize((SDL_Window*)mHandle, &top, &left, &bottom, &right);

  IntRect monitorBorderedRectangle = GetMonitorClientRectangle();

  monitorBorderedRectangle.X -= left;
  monitorBorderedRectangle.Y -= top;

  monitorBorderedRectangle.SizeX += right;
  monitorBorderedRectangle.SizeY += bottom;

  return monitorBorderedRectangle;
}

void ShellWindow::SetMonitorBorderedRectangle(const IntRect& monitorBorderedRectangle)
{
  int top = 0;
  int left = 0;
  int bottom = 0;
  int right = 0;

  SDL_GetWindowBordersSize((SDL_Window*)mHandle, &top, &left, &bottom, &right);

  IntRect monitorClientRectangle = monitorBorderedRectangle;

  monitorClientRectangle.X += left;
  monitorClientRectangle.Y += top;

  monitorClientRectangle.SizeX -= right;
  monitorClientRectangle.SizeY -= bottom;

  SetMonitorClientRectangle(monitorClientRectangle);

  mClientSize = monitorClientRectangle.Size();
}

IntVec2 ShellWindow::GetMinClientSize()
{
  return mMinClientSize;
}

void ShellWindow::SetMinClientSize(Math::IntVec2Param minSize)
{
  SDL_SetWindowMinimumSize((SDL_Window*)mHandle, minSize.x, minSize.y);
  mMinClientSize = minSize;
}

IntVec2 ShellWindow::MonitorToClient(Math::IntVec2Param monitorPosition)
{
  return monitorPosition - GetMonitorClientPosition();
}

IntVec2 ShellWindow::MonitorToBordered(Math::IntVec2Param monitorPosition)
{
  return ClientToBordered(MonitorToClient(monitorPosition));
}

IntVec2 ShellWindow::ClientToMonitor(Math::IntVec2Param clientPosition)
{
  return clientPosition + GetMonitorClientPosition();
}

IntVec2 ShellWindow::ClientToBordered(Math::IntVec2Param clientPosition)
{
  int top = 0;
  int left = 0;

  SDL_GetWindowBordersSize((SDL_Window*)mHandle, &top, &left, nullptr, nullptr);

  return IntVec2(clientPosition.x + left, clientPosition.y + top);
}

IntVec2 ShellWindow::BorderedToMonitor(Math::IntVec2Param borderedPosition)
{
  return ClientToMonitor(BorderedToClient(borderedPosition));
}

IntVec2 ShellWindow::BorderedToClient(Math::IntVec2Param borderedPosition)
{
  int top = 0;
  int left = 0;

  SDL_GetWindowBordersSize((SDL_Window*)mHandle, &top, &left, nullptr, nullptr);

  return IntVec2(borderedPosition.x - left, borderedPosition.y - top);
}

WindowStyleFlags::Enum ShellWindow::GetStyle()
{
  return mStyle.Field;
}

void ShellWindow::SetStyle(WindowStyleFlags::Enum style)
{
  mStyle = style;
  Error("Not implemented");
}

bool ShellWindow::GetVisible()
{
  return (SDL_GetWindowFlags((SDL_Window*)mHandle) & SDL_WINDOW_SHOWN) != 0;
}

void ShellWindow::SetVisible(bool visible)
{
  if (visible)
    SDL_ShowWindow((SDL_Window*)mHandle);
  else
    SDL_HideWindow((SDL_Window*)mHandle);
}

String ShellWindow::GetTitle()
{
  return SDL_GetWindowTitle((SDL_Window*)mHandle);
}

void ShellWindow::SetTitle(StringParam title)
{
  SDL_SetWindowTitle((SDL_Window*)mHandle, title.c_str());
}

WindowState::Enum ShellWindow::GetState()
{
  Uint32 flags = SDL_GetWindowFlags((SDL_Window*)mHandle);

  // Restore is never returned.
  if (flags & SDL_WINDOW_FULLSCREEN || flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
    return WindowState::Fullscreen;
  if (flags & SDL_WINDOW_MAXIMIZED)
    return WindowState::Maximized;
  if (flags & SDL_WINDOW_MINIMIZED)
    return WindowState::Minimized;

  return WindowState::Windowed;
}

void ShellWindow::SetState(WindowState::Enum windowState)
{
  switch (windowState)
  {
  case WindowState::Minimized:
  {
    SDL_MinimizeWindow((SDL_Window*)mHandle);
    break;
  }

  case WindowState::Windowed:
  {
    SDL_RestoreWindow((SDL_Window*)mHandle);
    break;
  }

  case WindowState::Maximized:
  {
    SDL_MaximizeWindow((SDL_Window*)mHandle);
    break;
  }

  case WindowState::Fullscreen:
  {
    SDL_SetWindowFullscreen((SDL_Window*)mHandle, SDL_WINDOW_FULLSCREEN_DESKTOP);
    break;
  }

  case WindowState::Restore:
  {
    SDL_RestoreWindow((SDL_Window*)mHandle);
    break;
  }
  default:
    break;
  }
}

void ShellWindow::SetMouseCapture(bool capture)
{
  SDL_CaptureMouse((SDL_bool)capture);
  mCapture = capture;
}

bool ShellWindow::GetMouseCapture()
{
  return mCapture;
}

void ShellWindow::TakeFocus()
{
  SDL_RaiseWindow((SDL_Window*)mHandle);
}

bool ShellWindow::HasFocus()
{
  return SDL_GetKeyboardFocus() == mHandle;
}

bool ShellWindow::GetImage(Image* image)
{
  Error("Not implemented");
  return false;
}

void ShellWindow::Close()
{
  SDL_Event e;
  memset(&e, 0, sizeof(e));
  e.type = SDL_WINDOWEVENT;
  e.window.timestamp = SDL_GetTicks();
  e.window.event = SDL_WINDOWEVENT_CLOSE;
  e.window.windowID = SDL_GetWindowID((SDL_Window*)mHandle);
  SDL_PushEvent(&e);
}

bool ShellWindow::HasOwnMinMaxExitButtons()
{
  return true;
}

} // namespace Zero
