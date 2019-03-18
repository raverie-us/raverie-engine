// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
ZeroThreadLocal bool gIsMainLoopSet = false;

void YieldToOs()
{
  // We don't currently do this because it requires white-listing the
  // entire call-stack (with mangled names) up to every call to YieldToOs.
  // Moreover, with SDL + EMTERPRETIFY it seems there are unresolvable
  // exceptions that get thrown, particularly an abort(-12).
  // emscripten_sleep(0);
}

void RunMainLoop(MainLoopFn callback, void* userData)
{
  StopMainLoop();

  ErrorIf(gIsMainLoopSet, "We should not have a main loop set since we called 'StopMainLoop'");

  gIsMainLoopSet = true;
  emscripten_set_main_loop_arg(callback, userData, 0, 0);
}

void StopMainLoop()
{
  if (!gIsMainLoopSet)
    return;

  emscripten_cancel_main_loop();
  gIsMainLoopSet = false;
}

} // namespace Zero
