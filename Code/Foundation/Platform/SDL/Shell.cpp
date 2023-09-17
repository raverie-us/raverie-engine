// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

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
    mOnDevicesChanged(nullptr),
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
