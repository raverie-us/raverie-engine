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


#if !defined(ZeroPlatformNoShellOpenFile)
void Shell::OpenFile(FileDialogInfo& config)
{
  SDL_ShowSimpleMessageBox(
      SDL_MESSAGEBOX_WARNING, "Unsupported", "The file open dialog is not yet supported in SDL", nullptr);
  if (config.mCallback)
    config.mCallback(config.mFiles, config.mUserData);
}
#endif

void Shell::ShowMessageBox(StringParam title, StringParam message)
{
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title.c_str(), message.c_str(), nullptr);
}

void UpdateResize(Shell* window, IntVec2Param clientSize)
{
  if (clientSize == window->mClientSize)
    return;

  window->mOnClientSizeChanged(clientSize, window);
  window->mClientSize = clientSize;
}

void Shell::Update()
{
  ZeroGetPrivateData(ShellPrivateData);

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

    case SDL_WINDOWEVENT:
    {
      Shell* window = GetShellFromSDLId(e.window.windowID);
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

Shell::Shell(Math::IntVec2Param clientSize) :
    mClientSize(clientSize),
    mCapture(false),
    mUserData(nullptr),
    mOnFocusChanged(nullptr),
    mOnClientSizeChanged(nullptr),
    mOnDevicesChanged(nullptr),
    mOnHitTest(nullptr),
    mOnInputDeviceChanged(nullptr)
{
}

void Shell::SetMouseCapture(bool capture)
{
  SDL_CaptureMouse((SDL_bool)capture);
  mCapture = capture;
}

bool Shell::GetMouseCapture()
{
  return mCapture;
}

bool Shell::HasFocus()
{
  return SDL_GetKeyboardFocus() == mHandle;
}

bool Shell::GetImage(Image* image)
{
  Error("Not implemented");
  return false;
}

void Shell::Close()
{
  SDL_Event e;
  memset(&e, 0, sizeof(e));
  e.type = SDL_WINDOWEVENT;
  e.window.timestamp = SDL_GetTicks();
  e.window.event = SDL_WINDOWEVENT_CLOSE;
  e.window.windowID = SDL_GetWindowID((SDL_Window*)mHandle);
  SDL_PushEvent(&e);
}

} // namespace Zero
