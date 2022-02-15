// MIT Licensed (see LICENSE.md).
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

} // namespace Zero
