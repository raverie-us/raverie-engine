// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
// On some platforms such as Emscripten we need to yield back to the operating
// system / browser. This is known as cooperative-multitasking.

// Yield back to the operating system or browser. This function does nothing on
// platforms that do not need it, however it may have serious build
// ramifications if used.
void YieldToOs();

// Let the operating system know that initial loading is done and we can display
void InitialLoadingCompleted();

typedef void (*MainLoopFn)(void* userData);

// Runs a loop in a continuous update until 'StopMainLoop' is called.
// Only one per thread is allowed. This will automatically call 'StopMainLoop'
// if another loop is currently running.
void RunMainLoop(MainLoopFn callback, void* userData);

// Stops a running main loop (may be called even when a loop is not running.
// Note that this may not return to the code that invoked 'RunMainLoop'
// depending on the platform.
void StopMainLoop();

} // namespace Zero
