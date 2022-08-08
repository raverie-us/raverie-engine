// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#define UseEmscriptenSleep

namespace Zero
{
#ifdef UseEmscriptenSleep
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
#else
void YieldToOs()
{
}

void RunMainLoop(MainLoopFn callback, void* userData)
{
  emscripten_set_main_loop_arg(callback, userData, 0, 0);
}

void StopMainLoop()
{
  emscripten_cancel_main_loop();
}
#endif

EM_JS(void, JSInitialLoadingCompleted, (), {
  window.parent.postMessage({type: 'initialLoadingCompleted'}, '*');
});

void InitialLoadingCompleted() {
  JSInitialLoadingCompleted();
}

} // namespace Zero
