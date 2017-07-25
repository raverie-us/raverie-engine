///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once


namespace Zero
{

DeclareBitField7(PhysicsSpaceDebugDrawFlags, DrawDebug, DrawOnTop, DrawBroadPhase, DrawConstraints,
                                             DrawSleeping, DrawSleepPreventors, DrawCenterMass);

}//namespace Zero
