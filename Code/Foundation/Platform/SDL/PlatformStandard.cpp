// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

SDL_GameController* cSDLGamePads[cMaxGamepads];
SDL_Haptic* cSDLHapticDevices[cMaxGamepads];

void PlatformLibrary::Initialize()
{
  Uint32 flags = SDL_INIT_EVERYTHING;
#if defined(WelderTargetOsEmscripten)
  SDL_SetHint(SDL_HINT_EMSCRIPTEN_ASYNCIFY, "0");
  SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
  emscripten_sample_gamepad_data();
  // https://github.com/emscripten-core/emscripten/issues/3139
  flags &= ~(SDL_INIT_TIMER | SDL_INIT_HAPTIC);
#endif

  if (SDL_Init(flags) != 0)
  {
    ZPrint("Unable to initialize SDL: %s\n", SDL_GetError());
    CrashHandler::FatalError(1);
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
