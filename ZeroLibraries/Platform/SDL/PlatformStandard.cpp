///////////////////////////////////////////////////////////////////////////////
///
/// \file OsShared.cpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

SDL_Cursor* gSDLCursors[SDL_NUM_SYSTEM_CURSORS] = { 0 };

SDL_GameController* cSDLGamePads[cMaxGamepads];
SDL_Haptic* cSDLHapticDevices[cMaxGamepads];

//**************************************************************************************************
void PlatformLibrary::Initialize()
{
  SDL_Init(SDL_INIT_EVERYTHING);

  for (size_t i = 0; i < SDL_NUM_SYSTEM_CURSORS; ++i)
    gSDLCursors[i] = SDL_CreateSystemCursor((SDL_SystemCursor)i);

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

//**************************************************************************************************
void PlatformLibrary::Shutdown()
{
  for (size_t i = 0; i < SDL_NUM_SYSTEM_CURSORS; ++i)
    SDL_FreeCursor(gSDLCursors[i]);

  SDL_Quit();
}

}//namespace Zero
