///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
// These directly match the flags in XInput (for other platforms use these bits)
enum GamepadButtonFlag
{
  DpadUp = 0x0001,
  DpadDown = 0x0002,
  DpadLeft = 0x0004,
  DpadRight = 0x0008,
  Start = 0x0010,
  Back = 0x0020,
  LThumb = 0x0040,
  RThumb = 0x0080,
  LShoulder = 0x0100,
  RShoulder = 0x0200,
  A = 0x1000,
  B = 0x2000,
  X = 0x4000,
  Y = 0x8000
};

// This structure mirrors the "XINPUT_GAMEPAD/XINPUT_STATE" structs
class GamepadState
{
public:
  size_t mPacketNumber;
  u16 mButtons;
  u8 mLTrigger;
  u8 mRTrigger;
  s16 mThumbLX;
  s16 mThumbLY;
  s16 mThumbRX;
  s16 mThumbRY;
};

ZeroShared bool AreGamepadsEnabled();

// Query the current state of a Gamepad (returns false if it fails or if the Gamepad is not connected)
// This function is responsible for ensuring that sticks within the dead-zone result in 0
ZeroShared bool GetGamepadState(size_t gamepadIndex, GamepadState* stateOut);
ZeroShared bool SetGamepadVibration(size_t gamepadIndex, float leftSpeed, float rightSpeed);

static const size_t cMaxGamepads = 4;

}//namespace Zero
