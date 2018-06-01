////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Dane Curbow
/// Copyright 2018, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

bool AreGamepadsEnabled()
{
  return false;
}

bool GetGamepadState(size_t gamepadIndex, GamepadState* stateOut)
{
  return false;
}

bool SetGamepadVibration(size_t gamepadIndex, float leftSpeed, float rightSpeed)
{
  return false;
}

}//namespace Zero
