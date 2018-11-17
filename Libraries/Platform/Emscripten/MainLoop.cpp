////////////////////////////////////////////////////////////////////////////////
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
// When running the browser you have to return control to the browser before it will
// update the render to the canvas (so special progress/loading screens will not work).
bool SupportsRenderingOutsideMainLoop = false;

ZeroThreadLocal bool gIsMainLoopSet = false;

void YieldToOs()
{
  // We don't currently do this because it requires white-listing the
  // entire call-stack (with mangled names) up to every call to YieldToOs.
  // Moreover, with SDL + EMTERPRETIFY it seems there are unresolvable
  // exceptions that get thrown, particularly an abort(-12).
  //emscripten_sleep(0);
}

void EmptyMainLoopFunction(void* userData)
{
}

void InitializeMainLoop()
{
  // We shouldn't have a main loop set yet, however it's technically possible.
  StopMainLoop();

  gIsMainLoopSet = true;
  emscripten_set_main_loop_arg(&EmptyMainLoopFunction, nullptr, 0, 0);
}

void RunMainLoop(MainLoopFn callback, void* userData)
{
  StopMainLoop();

  ErrorIf(gIsMainLoopSet, "We should not have a main loop set since we called 'StopMainLoop'");

  // This *MUST* come before because we are haulting execution here.
  gIsMainLoopSet = true;
  emscripten_set_main_loop_arg(callback, userData, 0, 1);
}

void StopMainLoop()
{
  if (!gIsMainLoopSet)
    return;

  emscripten_cancel_main_loop();
  gIsMainLoopSet = false;
}

}// namespace Zero
