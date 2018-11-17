///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Get a animation time for the cog to debug draw. Typically just comes from the space's total time passed.
real GetAnimationTime(Cog* cog);
// There's currently no disc debug drawing so hack this in until Nate finishes his graphics refactor
void DebugDrawDisc(Vec3Param center, Vec3Param axis0, Vec3Param axis1, real radius0, real radius1);
// Draw a sub-divided ring with arrow heads (right-handed rule). A time can be provided to animate the ring.
void DrawRing(Vec3Param center, Vec3Param axis, real radius, size_t subDivisions, real time, ByteColor color);

// Get scaled vector lengths of the penumbra values. Scales to account for min/max getting too
// close for good visibility. Positive force values are assumed to be outward.
void GetPenumbraDebugDrawValues(real minDistance, real maxDistance, real& minForce, real& maxForce);

// Computes a cylinder for a given support shape with the specified primary axis.
Cylinder GetSupportShapeCylinder(const Intersection::SupportShape& supportShape, Vec3Param primaryAxis);
// Computes a cylinder for the given cog and the primary axis of the cylinder. Used currently in flow effects
Cylinder GetCogCylinder(Cog* cog, Vec3Param primaryAxis);

} // namespace Zero
