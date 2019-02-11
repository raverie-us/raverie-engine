// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
// We use this bool globally to set whether certain operations should be as
// deterministic as possible. For example, this means that frame timers would
// return fixed times, guid generation will generate guids in a deterministic
// fashion (non random), etc. This should be set as early in the program as
// possible (default false). This is used by the UnitTestSystem to ensure
// recording and playback are the same.
extern bool gDeterministicMode;
} // namespace Zero
