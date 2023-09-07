// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

SDL_GameController* cSDLGamePads[cMaxGamepads];
SDL_Haptic* cSDLHapticDevices[cMaxGamepads];

void PlatformLibrary::Initialize()
{
  Uint32 flags = SDL_INIT_EVERYTHING;

  if (SDL_Init(flags) != 0)
  {
    ZPrint("Unable to initialize SDL: %s\n", SDL_GetError());
    abort();
  }

  // We don't want the back buffer to be multi-sampled because we can't blit a
  // frame buffer to it.
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

  // Initialize all connected gamepads for use
  for (int i = 0; i < cMaxGamepads; ++i)
    cSDLGamePads[i] = nullptr;

  int activeGamepads = Math::Clamp(SDL_NumJoysticks(), 0, (int)cMaxGamepads);
  for (int i = 0; i < activeGamepads; ++i)
  {
    // Open all currently connected game controllers
    cSDLGamePads[i] = SDL_GameControllerOpen(i);
  }

  // Attempt to initialize all haptic feedback devices on the gamepads
  for (int i = 0; i < cMaxGamepads; ++i)
    cSDLHapticDevices[i] = nullptr;

  for (int i = 0; i < cMaxGamepads; ++i)
  {
    SDL_GameController* gamepad = cSDLGamePads[i];
    if (gamepad)
    {
      SDL_Joystick* joystick = SDL_GameControllerGetJoystick(gamepad);
      cSDLHapticDevices[i] = SDL_HapticOpenFromJoystick(joystick);
    }
  }
}

void PlatformLibrary::Shutdown()
{
  SDL_Quit();
}

void StackHandle::Close()
{
}

} // namespace Zero
