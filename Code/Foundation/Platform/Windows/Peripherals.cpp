// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
typedef DWORD(WINAPI* XInputGetState_T)(DWORD dwUserIndex, XINPUT_STATE* pState);
typedef DWORD(WINAPI* XInputSetState_T)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);
XInputGetState_T XInputGetState_P = NULL;
XInputSetState_T XInputSetState_P = NULL;
HMODULE XInputModule = NULL;
const float MaxRumble = 65535.0f;

// Use XInput 9.1.0 for compatibility with Windows 7 and Windows Vista
#define XINPUT_DLL_VER_9_1_0 L"xinput9_1_0.dll"

void InitializeGamepad()
{
  // Load the library instead of linked in case XInput is not installed
  XInputModule = LoadLibrary(XINPUT_DLL_VER_9_1_0);
  if (XInputModule)
  {
    ZPrint("Using XINPUT for gamepads.\n");
    XInputGetState_P = (XInputGetState_T)GetProcAddress(XInputModule, "XInputGetState");
    XInputSetState_P = (XInputSetState_T)GetProcAddress(XInputModule, "XInputSetState");
  }
  else
  {
    ZPrint("Failed to load XINPUT. Gamepads will not be available.\n");
  }
}

static_assert(sizeof(XINPUT_STATE) == sizeof(GamepadState),
              "The size of the state in Platform.hpp must match the size of "
              "XINPUT_STATE");

bool AreGamepadsEnabled()
{
  return XInputModule != nullptr;
}

bool GetGamepadState(size_t gamepadIndex, GamepadState* stateOut)
{
  if (!XInputModule || gamepadIndex >= cMaxGamepads)
    return false;

  bool result = (*XInputGetState_P)(gamepadIndex, (XINPUT_STATE*)stateOut) == ERROR_SUCCESS;

  if (!result)
    return false;

  float LX = stateOut->mThumbLX;
  float LY = stateOut->mThumbLY;
  float RX = stateOut->mThumbRX;
  float RY = stateOut->mThumbRY;

  // Check for dead zone
  if (sqrt(LX * LX + LY * LY) < float(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))
  {
    stateOut->mThumbLX = 0;
    stateOut->mThumbLY = 0;
  }

  if (sqrt(RX * RX + RY * RY) < float(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))
  {
    stateOut->mThumbRX = 0;
    stateOut->mThumbRY = 0;
  }

  return result;
}

bool SetGamepadVibration(size_t gamepadIndex, float leftSpeed, float rightSpeed)
{
  if (!XInputModule || gamepadIndex >= cMaxGamepads)
    return false;

  XINPUT_VIBRATION zerovibration;
  zerovibration.wLeftMotorSpeed = (WORD)(leftSpeed * MaxRumble);
  zerovibration.wRightMotorSpeed = (WORD)(rightSpeed * MaxRumble);
  return (*XInputSetState_P)(gamepadIndex, &zerovibration) == ERROR_SUCCESS;
}

} // namespace Zero
