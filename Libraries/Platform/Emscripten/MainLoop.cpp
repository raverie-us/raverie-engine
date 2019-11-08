// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
bool gYieldToOsEnabled = true;
void YieldToOs()
{
  emscripten_sleep(0);
}

ZeroThreadLocal bool gStopMainLoop = false;

void RunMainLoop(MainLoopFn callback, void* userData)
{
  while (!gStopMainLoop)
    callback(userData);

  gStopMainLoop = false;
}

void StopMainLoop()
{
  gStopMainLoop = true;
}

} // namespace Zero
