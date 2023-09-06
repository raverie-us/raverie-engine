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

} // namespace Zero
