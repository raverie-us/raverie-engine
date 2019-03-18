// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
ZeroThreadLocal bool gStopMainLoop = false;

void YieldToOs()
{
  // Most platforms aren't cooperatively multi-threaded.
}

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
