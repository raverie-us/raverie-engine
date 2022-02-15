// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

// Values taken from XINPUT dead zone defines
#define GAMEPAD_LEFT_THUMB_DEADZONE 7849
#define GAMEPAD_RIGHT_THUMB_DEADZONE 8689
const float MaxRumble = 65535.0f;

extern SDL_GameController* cSDLGamePads[];
extern SDL_Haptic* cSDLHapticDevices[];

SDL_HapticEffect cSDLHapticEffects[cMaxGamepads];

bool AreGamepadsEnabled()
{
#if defined(WelderTargetOsEmscripten)
  emscripten_sample_gamepad_data();
#endif

  int activeGamepads = SDL_NumJoysticks();
  for (int i = 0; i < activeGamepads; ++i)
  {
    // Returns true given device at the index is supported by the SDL game
    // controller interface
    if (SDL_IsGameController(i))
      return true;
  }
  return false;
}

void SetButtonState(GamepadState* stateOut,
                    SDL_GameController* gamepad,
                    GamepadButtonFlag zeroButtonFlag,
                    SDL_GameControllerButton sdlButtonFlag)
{
  int bitShift = (int)Math::Log2((real)zeroButtonFlag);
  stateOut->mButtons =
      ((stateOut->mButtons & zeroButtonFlag) | (SDL_GameControllerGetButton(gamepad, sdlButtonFlag) << bitShift));
}

SDL_GameController* GetGamePad(size_t gamepadIndex)
{
  // Check if the device at the given index is a valid gamepad for SDL
  if (!SDL_IsGameController(gamepadIndex) || gamepadIndex > cMaxGamepads)
    return nullptr;

  SDL_GameController* gamepad = cSDLGamePads[gamepadIndex];

  // Check if the gamepad is active, if not attempt to activate it for use
  if (!SDL_GameControllerGetAttached(gamepad))
  {
    gamepad = SDL_GameControllerOpen(gamepadIndex);
    if (!gamepad)
      return nullptr;
    else
      cSDLGamePads[gamepadIndex] = gamepad;
  }

  return gamepad;
}

bool GetGamepadState(size_t gamepadIndex, GamepadState* stateOut)
{
#if defined(WelderTargetOsEmscripten)
  emscripten_sample_gamepad_data();
#endif

  SDL_GameController* gamepad = GetGamePad(gamepadIndex);
  if (!gamepad)
    return false;

  // Get the state of the buttons
  SetButtonState(stateOut, gamepad, GamepadButtonFlag::A, SDL_CONTROLLER_BUTTON_A);
  SetButtonState(stateOut, gamepad, GamepadButtonFlag::B, SDL_CONTROLLER_BUTTON_B);
  SetButtonState(stateOut, gamepad, GamepadButtonFlag::X, SDL_CONTROLLER_BUTTON_X);
  SetButtonState(stateOut, gamepad, GamepadButtonFlag::Y, SDL_CONTROLLER_BUTTON_Y);
  SetButtonState(stateOut, gamepad, GamepadButtonFlag::Back, SDL_CONTROLLER_BUTTON_BACK);
  SetButtonState(stateOut, gamepad, GamepadButtonFlag::Start, SDL_CONTROLLER_BUTTON_START);
  SetButtonState(stateOut, gamepad, GamepadButtonFlag::LThumb, SDL_CONTROLLER_BUTTON_LEFTSTICK);
  SetButtonState(stateOut, gamepad, GamepadButtonFlag::RThumb, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
  SetButtonState(stateOut, gamepad, GamepadButtonFlag::LShoulder, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
  SetButtonState(stateOut, gamepad, GamepadButtonFlag::RShoulder, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
  SetButtonState(stateOut, gamepad, GamepadButtonFlag::DpadUp, SDL_CONTROLLER_BUTTON_DPAD_UP);
  SetButtonState(stateOut, gamepad, GamepadButtonFlag::DpadDown, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
  SetButtonState(stateOut, gamepad, GamepadButtonFlag::DpadLeft, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
  SetButtonState(stateOut, gamepad, GamepadButtonFlag::DpadRight, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

  // Get the state of the thumb sticks
  s16 thumbLX = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_LEFTX);
  s16 thumbLY = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_LEFTY);
  s16 thumbRX = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_RIGHTX);
  s16 thumbRY = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_RIGHTY);

  // Get the state of the triggers. The function returns an s16, but triggers
  // will only ever return a value 0 to 32767.
  stateOut->mLTrigger = (u8)SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
  stateOut->mRTrigger = (u8)SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

  // Check for dead zone
  if (sqrt(thumbLX * thumbLX + thumbLY * thumbLY) < float(GAMEPAD_LEFT_THUMB_DEADZONE))
  {
    thumbLX = 0;
    thumbLY = 0;
  }

  if (sqrt(thumbRX * thumbRX + thumbRY * thumbRY) < float(GAMEPAD_RIGHT_THUMB_DEADZONE))
  {
    thumbRX = 0;
    thumbRY = 0;
  }

  stateOut->mThumbLX = thumbLX;
  stateOut->mThumbLY = thumbLY;
  stateOut->mThumbRX = thumbRX;
  stateOut->mThumbRY = thumbRY;

  return true;
}

SDL_Haptic* GetSDLHapticDevice(size_t gamepadIndex)
{
  SDL_GameController* gamepad = GetGamePad(gamepadIndex);
  if (!gamepad)
    return nullptr;

  // Check if the haptic device is active, if not attempt to activate it for use
  SDL_Haptic* hapticDevice = cSDLHapticDevices[gamepadIndex];
  if (!hapticDevice)
  {
    SDL_Joystick* joystick = SDL_GameControllerGetJoystick(gamepad);
    hapticDevice = SDL_HapticOpenFromJoystick(joystick);
    if (!hapticDevice)
      return nullptr;
    else
      cSDLHapticDevices[gamepadIndex] = hapticDevice;
  }

  return hapticDevice;
}

bool SetGamepadVibration(size_t gamepadIndex, float leftSpeed, float rightSpeed)
{
  SDL_GameController* gamepad = GetGamePad(gamepadIndex);
  if (!gamepad)
    return false;

  SDL_Haptic* hapticDevice = GetSDLHapticDevice(gamepadIndex);

  // If there are any currently running effects stop them
  SDL_HapticStopAll(hapticDevice);

  // Setup the rumble effect for the given gamepad
  SDL_HapticEffect& rumble = cSDLHapticEffects[gamepadIndex];
  SDL_memset(&rumble, 0, sizeof(SDL_HapticLeftRight));
  rumble.type = SDL_HAPTIC_LEFTRIGHT;
  rumble.leftright.length = SDL_HAPTIC_INFINITY;

  // The left motor is the larger of the two (in the xbox controllers)
  leftSpeed = Math::Clamp(leftSpeed, 0.0f, 1.0f);
  rightSpeed = Math::Clamp(rightSpeed, 0.0f, 1.0f);
  rumble.leftright.large_magnitude = (Uint16)(leftSpeed * MaxRumble);
  rumble.leftright.small_magnitude = (Uint16)(rightSpeed * MaxRumble);

  // Setup the rumble effect
  int effectId = SDL_HapticNewEffect(hapticDevice, &rumble);
  // Run the rumble effect until set to a different value
  SDL_HapticRunEffect(hapticDevice, effectId, SDL_HAPTIC_INFINITY);

  return true;
}

} // namespace Zero
