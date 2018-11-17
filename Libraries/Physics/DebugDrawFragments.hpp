///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{

void DrawAnchors(WorldAnchorAtom& anchors, Collider* obj0, Collider* obj1);
void DrawBasis(Mat3Param basis, Vec3Param pos, real size = real(1.0));
void DrawArc(Vec3Param center, Vec3Param normal, Vec3Param axis, real radiusA, real radiusB,
  real minAngle, real maxAngle, bool drawBoundaries, ByteColor color, real radianStepSize, bool leftHanded = false);
void DrawHinge(Joint* joint, AnchorAtom& anchors, Mat3Param basis0, Mat3Param basis1, uint defaultPerspective = 0);

}//namespace Physics

}//namespace Zero
