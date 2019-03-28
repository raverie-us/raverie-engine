// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

DeclareBitField7(PhysicsSpaceDebugDrawFlags,
                 DrawDebug,
                 DrawOnTop,
                 DrawBroadPhase,
                 DrawConstraints,
                 DrawSleeping,
                 DrawSleepPreventors,
                 DrawCenterMass);

} // namespace Zero
