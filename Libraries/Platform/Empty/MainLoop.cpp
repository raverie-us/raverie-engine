// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
void YieldToOs()
{
  // Most platforms aren't cooperatively multi-threaded.
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
